#include <Arduino.h>
#include <freertos/queue.h>
#include <Wire.h>
#include <EEPROM.h>

QueueHandle_t interruptQueue;

#include <TimerOne.h>
#include <GyverEncoder.h>
Encoder enc1(34, 35, 32);

#include <GyverStepper2.h>
GStepper2<STEPPER2WIRE> pull_stepper(800, 33, 25);
GStepper2<STEPPER2WIRE> winding_stepper(800, 26, 27);
GStepper2<STEPPER2WIRE> packer_stepper(800, 14, 12);
#define microsteps 8
// #define windingGearbox 4

#include <GyverPID.h>
GyverPID pullPID(0, 0, 0);
GyverPID windingPID(0, 0, 0);

#include <DigitalMicrometer.h>
Micrometer micrometer(16, 4);

#define packerEndSensor 13
#define tensionSensor 15
#define modeSensor 2

const char* status_decode[] = {"[Wait]   ", "[Work]   ", "[Done]   ", "[Pause]  ", "[Prepare]", "[Error]  "};
#define Wait 0
#define Work 1
#define Done 2
#define Pause 3
#define Prepare 4
#define Error 5

const char* mode_decode[] = {"Off   ", "Manual", "Auto  "};
#define Mode_Off 0
#define Mode_Manual 1
#define Mode_Auto 2


struct StatusData {
  bool type = 0;
  int time = 0;
  int status = 0;                               // текущее состояние станка
  int mode = 0;                                 // режим работы
  float cur_diameter = 0;                       // текущий диаметр прутка
  float err_diameter = 0;                       // погрешность диаметра прутка
  unsigned long timeWork = 0;                   // время работы
  unsigned long leftWorkTime = 0;               // осталось времени работы
  String leftWorkTimeUTC;                 // осталось времени работы (часы и минуты)
  float produced_speed = 0;                     // скорость производства в метрах в секунду
  float produced_filament = 0;                 // произведено метров прутка
  unsigned long produced_gramm_filament = 0;    // произведено грамм прутка      
};

struct SettingsData {
  float set_diameter = 1.75;
  float tension = 500;
  float step = 1.75;
  float pull_P = 20;
  float pull_I = 15;
  float pull_D = 5;
  float winding_P = 5;
  float winding_I = 2;
  float winding_D = 1;
  float wheel_diameter = 80;
  float pullGearbox = 2.5;
  float windingGearbox = 4;
  int weight = 50;
  int material = 1;
  float manual_speed = 2;
  unsigned long produced_filament = 0;
};

StatusData status_data;
SettingsData settings_data;

// Списки для параметров
// const char* weightList[] = {"50kg", "25kg", "10kg"};
const char* materialList[] = {"Abs", "Petg", "Pla"};

struct Material {
  const char* name;
  float density;
};

Material materials[] = {
  {"Abs", 1.1},
  {"Petg", 12.7},
  {"Pla", 1.24}
};

uint32_t tmr0 = 0;
uint32_t tmr1 = 0;
uint32_t tmr2 = 0;
uint32_t worktmr = 0;
long FPS;

float weight_gramm_metr;
bool packer_dir = false;
bool StartWorkflag = false;
bool EndWorkFlag = false;


#include <LcdManager.h>
LcdManager::MenuItem menu[] = {
  {"Settings", -1, 1, 5, NULL, NULL, 0, 0, NULL, 0},
      {"Presets", 0, 2, 2, NULL, NULL, 0, 0, NULL, 0},
          {"Weight", 1, -1, 0, NULL, &settings_data.weight, 0, 50, NULL, 1},
          {"Material", 1, -1, 0, NULL, &settings_data.material, 0, 2, materialList, 1}, 
      {"Pull", 0, 5, 4, NULL, NULL, 0, 0, NULL, 0},
          {"PID", 4, 6, 3, NULL, NULL, 0, 0, NULL, 0},
              {"P", 5, -1, 0, &settings_data.pull_P, NULL, 0, 999, NULL, 1},
              {"I", 5, -1, 0, &settings_data.pull_I, NULL, 0, 999, NULL, 1}, 
              {"D", 5, -1, 0, &settings_data.pull_D, NULL, 0, 999, NULL, 1},
          {"Set diameter", 4, -1, 0, &settings_data.set_diameter, NULL, 0, 10, NULL, 0.1},
          {"Wheel dm", 4, -1, 0, &settings_data.wheel_diameter, NULL, 0, 999, NULL, 1},
          {"Reduction", 4, -1, 0, &settings_data.pullGearbox, NULL, 0, 50, NULL, 0.1},
      {"Winding", 0, 13, 3, NULL, NULL, 0, 0, NULL, 0},
          {"PID", 12, 12, 3, NULL, NULL, 0, 0, NULL, 0},
              {"P", 13, -1, 0, &settings_data.winding_P, NULL, 0, 999, NULL, 1},
              {"I", 13, -1, 0, &settings_data.winding_P, NULL, 0, 999, NULL, 1}, 
              {"D", 13, -1, 0, &settings_data.winding_P, NULL, 0, 999, NULL, 1},
          {"Set tension", 12, -1, 0, &settings_data.tension, NULL, 0, 999, NULL, 1},
          {"Reduction", 12, -1, 0, &settings_data.windingGearbox, NULL, 0, 50, NULL, 0.1},
      {"Packer", 0, 20, 1, NULL, NULL, 0, 0, NULL, 0},
          {"Step", 19, -1, 0, &settings_data.step, NULL, 0, 10, NULL, 0.1},
      {"Manual set", 0, 22, 1, NULL, NULL, 0, 0, NULL, 0},
          {"Speed", 21, -1, 0, &settings_data.manual_speed, NULL, 0, 15, NULL, 0.1},
};

LcdManager::Main lcd_main[] = {
  {"Status", 0, 0, LcdManager::Main::INT, {.intData = &status_data.status}, status_decode, false, NULL, NULL, false},
  // {"Time", 0, 1, LcdManager::Main::INT, {.intData = &status_data.time}, NULL, false, NULL, NULL, false}, 
  {"Mode:", 0, 3, LcdManager::Main::INT, {.intData = &status_data.mode}, mode_decode, false, NULL, NULL, false},
  {"set:", 11, 0, LcdManager::Main::FLOAT, {.floatData = &settings_data.set_diameter}, NULL, true, NULL, NULL, false},
  {"cur:", 11, 1, LcdManager::Main::FLOAT, {.floatData = &status_data.cur_diameter}, NULL, true, &micrometer.error, "ERR01", false},
  {"err:", 11, 2, LcdManager::Main::FLOAT, {.floatData = &status_data.err_diameter}, NULL, true, &micrometer.error, "ERR01", false},
  {"LeftTime", 13, 3, LcdManager::Main::STRING, {.textData = &status_data.leftWorkTimeUTC}, NULL, false, &StartWorkflag, " --H--M", true},
};

LcdManager lcd_manager(0x27, 20, 4,
                       lcd_main, sizeof(lcd_main)/sizeof(LcdManager::Main),
                       menu, sizeof(menu)/sizeof(LcdManager::MenuItem));


void save() 
{
  EEPROM.put(0, settings_data);
  EEPROM.commit();
}

void read()
{
  EEPROM.get(0, settings_data);
}

void dir() {
  static bool flag;
  if (!digitalRead(packerEndSensor) && !flag){ 
    packer_dir = !packer_dir;
    packer_stepper.reverse(packer_dir);
    flag = true;
  }
  if (digitalRead(packerEndSensor)){
    flag = false;
  }
}

// bool packerHome(bool start = false) {
//   static bool flag;
//   if (start = true) {
//     flag = false;
//     packer_dir = 0;
//     packer_stepper.reverse(packer_dir);
//     packer_stepper.setSpeed(6800);
//   }
//   if (!digitalRead(packerEndSensor)){
//     flag = true;
//   }
//   return flag;
// }

String secondToUTC(unsigned long second) {
  String res = "";
  res += second / 3600;
  res += "H";
  second %= 3600;
  if (second / 60 < 10) res += "0";
  res += second / 60;
  res += "M";
  return res;
}

void encController() {
  if (enc1.isTurn()){
    if (enc1.isRight()) lcd_manager.Right();
    else if (enc1.isLeft()) lcd_manager.Left(); 
    else if (enc1.isRightH()) lcd_manager.RightH();
    else if (enc1.isLeftH()) lcd_manager.LeftH();
  }

  if (enc1.isSingle() && !enc1.isHold()) {
    lcd_manager.press();
  }

  if (enc1.isHolded()) {
    lcd_manager.Holded();
  }
}

void startWork() {
  worktmr = millis();
  // packerHome(true);
}

bool checkError() {
  return micrometer.error;
}

int status() {
  status_data.mode = map(analogRead(modeSensor), 0, 4096, 0, 3);

  if (!checkError()) {
    if (StartWorkflag) {
      if (status_data.mode == Mode_Off) {
        status_data.status = Pause;
      } else if (status_data.mode == Mode_Manual) {
        status_data.status = Prepare;
        status_data.produced_filament = 0;
        EndWorkFlag = false;
        StartWorkflag = false;
      } else if (status_data.mode == Mode_Auto) {
        if (!EndWorkFlag) status_data.status = Work;
        else status_data.status = Done;
      }
    } else {
      if (status_data.mode == Mode_Off) {
        status_data.status = Wait;
      } else if (status_data.mode == Mode_Manual) {
        status_data.status = Prepare;
        
      } else if (status_data.mode == Mode_Auto) {
        startWork();
        StartWorkflag = true;
      }
    }
  } else {
    status_data.status = Error;
  }

  return status_data.status;
}

void ITR() {
  enc1.tick();
  pull_stepper.tick();
  winding_stepper.tick(); 
  packer_stepper.tick();
}

void setSettings() {
  pullPID.setLimits(0, 200*microsteps*4);
  pullPID.Kp = settings_data.pull_P;
  pullPID.Ki = settings_data.pull_I;
  pullPID.Kd = settings_data.pull_D;
  pullPID.setpoint = settings_data.set_diameter;

  windingPID.setLimits(0, 200*microsteps*4);
  windingPID.Kp = settings_data.pull_P;
  windingPID.Ki = settings_data.pull_I;
  windingPID.Kd = settings_data.pull_D;
  windingPID.setpoint = settings_data.tension;

  weight_gramm_metr = PI * pow(1.75 / 2, 2) * materials[settings_data.material].density;
}

void setup()
{
  Serial.begin(115200);
  micrometer.begin();
  lcd_manager.init();

  EEPROM.begin(512);
  // save();
  read();

  pinMode(packerEndSensor, INPUT);
  pinMode(tensionSensor, INPUT);

  enc1.setType(TYPE2);

  Timer1.initialize(25);            // установка таймера на каждые 1000 микросекунд (= 1 мс)
  Timer1.attachInterrupt(ITR); 

  setSettings();
}


void loop()
{
  encController();
  if (millis() - tmr0 > 50) {
    
    lcd_manager.tick();
    micrometer.tick();
    tmr0 = millis();
  }  

  if (millis() - tmr1 > 50) {
    status_data.err_diameter = abs(settings_data.set_diameter - status_data.cur_diameter);
    status_data.cur_diameter = micrometer.GetValue();
    status_data.time = millis() / 1000;

    if (status() != Error) {
      if (status_data.mode == Mode_Manual) {
        pullPID.input = status_data.cur_diameter;
        pull_stepper.setSpeed(pullPID.getResult());
      } else if (status_data.mode == Mode_Auto && EndWorkFlag == false) {// && packerHome()
        dir();
        pullPID.input = status_data.cur_diameter;
        windingPID.input = map(analogRead(tensionSensor), 0, 4096, 0, 1000);

        pull_stepper.setSpeed(pullPID.getResult());
        winding_stepper.setSpeed(windingPID.getResult());
        packer_stepper.setSpeed(windingPID.getResult() * (settings_data.step / microsteps) * settings_data.windingGearbox);
      } else {
        pull_stepper.setSpeed(0);
        winding_stepper.setSpeed(0);
        packer_stepper.setSpeed(0);      
      }
    } else {
      pull_stepper.setSpeed(0);
      winding_stepper.setSpeed(0);
      packer_stepper.setSpeed(0);      
    }

    if (lcd_manager.settingChanged) {
      setSettings();                // обновляем настройки
      save();                       // сохраняем настройки
      Serial.println("сохранение...");
      lcd_manager.settingChanged = false;
    }

    // Serial.print(windingPID.getResult());
    // Serial.print("  ");
    // Serial.print(windingPID.getResult() * (settings_data.step / 8));
    // Serial.print("  ");
    // Serial.println(map(analogRead(tensionSensor), 0, 4096, 0, 1000));
    tmr1 = millis();
  }

  if (millis() - tmr2 > 1000) {
    if (status_data.status != Error && status_data.status == Work && EndWorkFlag == false) {// && packerHome()
      status_data.timeWork = (millis() - worktmr) / 1000;
      status_data.produced_speed = pullPID.getResult() / (microsteps*200) / settings_data.pullGearbox * (settings_data.wheel_diameter / 1000.0f * PI);
      status_data.produced_filament += status_data.produced_speed;
      status_data.produced_gramm_filament = weight_gramm_metr * status_data.produced_filament;
      status_data.leftWorkTime = (settings_data.weight * 1000 - status_data.produced_gramm_filament) / (weight_gramm_metr * status_data.produced_speed);
      status_data.leftWorkTimeUTC = leftPadString(secondToUTC(status_data.leftWorkTime), 7);
      if (status_data.leftWorkTime == 0) {
        EndWorkFlag = true;
      }
    }
    Serial.print(status_data.timeWork);
    Serial.print("  ");
    Serial.print(pullPID.getResult());
    Serial.print("  ");
    Serial.print(status_data.produced_speed);
    Serial.print("  ");
    Serial.print(status_data.produced_filament);
    Serial.print("  ");
    Serial.print(status_data.produced_gramm_filament);
    Serial.print("  ");
    Serial.print(status_data.leftWorkTime);
    Serial.print("  ");
    Serial.print(secondToUTC(status_data.leftWorkTime));
    Serial.print("  ");
    Serial.print(status_decode[status_data.status]);
    Serial.print("  ");
    Serial.print(packer_dir);
    Serial.print("  ");
    Serial.print(StartWorkflag);
   
    Serial.print("  ");
    Serial.println(FPS);
    tmr2 = millis();
    FPS = 0;
  }

  FPS++;
}