#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <TimerOne.h>

#include <GyverEncoder.h>
Encoder enc1(34, 35, 32);

#define enc 15

// #include "Stepper.h"
// Stepper rewinder(400, 26, 27);
#include <GyverStepper2.h>
GStepper2<STEPPER2WIRE> rewinder(200, 26, 27);
GStepper2<STEPPER2WIRE> packer_stepper(800, 14, 12);
// #define microsteps 8

// #include <GyverPID.h>
// GyverPID rewinderPID(0, 2, 500);

#include <button.h>
button packerEndSensor(13);

button startSwitch(4);
button stopSwitch(16);

button nextStepSwitch(17);
button prevStepSwitch(5);

button lockSwitch(18);
button unlockSwitch(19);

const char* status_decode[] = {"[Wait]   ", "[Work]   ", "[Done]   ", "[Pause]  ", "[Prepare]", "[Error]  "};
#define Wait 0
#define Work 1
#define Done 2
#define Pause 3
#define Prepare 4
#define Error 5

struct StatusData {
  int status = 0;                               // текущее состояние станка
  int task = 0;                                 // текущая задача: 0 - нет задачи, 1 - намотка катушки, 2 - поворот, 3 - зажим катушки, 4 - разжим катушки
  unsigned long timeWork = 0;                   // время работы
  unsigned long leftWorkTime = 0;               // осталось времени работы
  String leftWorkTimeUTC;                       // осталось времени работы (часы и минуты)     
};

struct SettingsData {
  float step = 1.75;
  float pullGearbox = 2.5;
  float windingGearbox = 4;
  float weight = 0.1;
  int material = 1;
  int manual_pt = 3;                            // количество ручных точек
  int maxSpeed = 2000;                          // об/мин
  int startAccelerate = 200;                    // об/c^3
  int endAccelerate = 200;                      // об/c^3
  float wheel_diameter = 15;
};

StatusData status_data;
SettingsData settings_data;

const char* materialList[] = {"Abs", "Petg", "Pla"};

struct Material {
  const char* name;
  float density;
};

Material materials[] = {
  {"Abs", 1.1},
  {"Petg", 1.27},
  {"Pla", 1.24}
};

uint32_t tmr0 = 0;
uint32_t tmr1 = 0;
uint32_t tmr2 = 0;
long FPS;
long turn;
long target;

double speed = 0;

bool packer_dir = false;
bool StartWorkflag = false;
bool EndWorkFlag = false;

#include <LcdManager.h>
LcdManager::MenuItem menu[] = {
  {"Settings", -1, 1, 5, NULL, NULL, 0, 0, NULL, 0},
      {"Presets", 0, 2, 2, NULL, NULL, 0, 0, NULL, 0},
          {"Weight", 1, -1, 0, &settings_data.weight, NULL, 0, 50, NULL, 0.1},
          {"Material", 1, -1, 0, NULL, &settings_data.material, 0, 2, materialList, 1}, 
      {"Speed", 0, 5, 1, NULL, NULL, 0, 0, NULL, 0},
          {"Reduction", 4, -1, 0, &settings_data.pullGearbox, NULL, 0, 50, NULL, 0.1},
      {"Packer", 0, 7, 1, NULL, NULL, 0, 0, NULL, 0},
          {"Step", 6, -1, 0, &settings_data.step, NULL, 0, 10, NULL, 0.1},
      {"Manual set", 0, 9, 1, NULL, NULL, 0, 0, NULL, 0},
          {"Points", 8, -1, 0, NULL, &settings_data.manual_pt, 0, 15, NULL, 1},
};

LcdManager::Main lcd_main[] = {
  {"Status", 0, 0, LcdManager::Main::INT, {.intData = &status_data.status}, status_decode, false, NULL, NULL, false, NULL},
  {"Type:", 0, 2, LcdManager::Main::INT, {.intData = &settings_data.material}, materialList, true, NULL, NULL, false, NULL},
  {"Weight:", 0, 3, LcdManager::Main::FLOAT, {.floatData = &settings_data.weight}, NULL, true, NULL, NULL, false, 2},
  {"LeftTime", 13, 3, LcdManager::Main::STRING, {.textData = &status_data.leftWorkTimeUTC}, NULL, false, &StartWorkflag, " --H--M", true, NULL},
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

// void dir() {
//   static bool flag;
//   if (!digitalRead(packerEndSensor) && !flag){ 
//     packer_dir = !packer_dir;
//     packer_stepper.reverse(packer_dir);
//     flag = true;
//   }
//   if (digitalRead(packerEndSensor)){
//     flag = false;
//   }
// }

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

bool checkError() {
  return 0;
}

void setTask() {

}

int status() {
  if (stopSwitch.click()) {
    status_data.task = 0;
    status_data.status = Pause;        
  }

  if (!checkError()) {
    if (!status_data.task) {
      
      if (startSwitch.click()) {
        status_data.task = 1;
        turn = 0;
        float weight_gramm_metr = PI * pow(1.75 / 2, 2) * materials[settings_data.material].density;
        target = ((settings_data.weight * 1000) / weight_gramm_metr) / (settings_data.wheel_diameter / 100. * PI);
        ledcAttachPin(26, 0);
        status_data.status = Work;
      } else if (nextStepSwitch.click()) {
        status_data.task = 2;
        rewinder.setTargetDeg(360/settings_data.manual_pt, RELATIVE);
        status_data.status = Work;
      } else if (prevStepSwitch.click()) {
        status_data.task = 2;
        rewinder.setTargetDeg(-360/settings_data.manual_pt, RELATIVE);
        status_data.status = Work;
      } else if (lockSwitch.click()) {
        status_data.task = 3;
        rewinder.setTargetDeg(360*10, RELATIVE);
        status_data.status = Prepare;
      } else if (unlockSwitch.click()) {
        status_data.task = 4;
        rewinder.setTargetDeg(-360*10, RELATIVE);
        status_data.status = Prepare;
      }
    }
  } else {
    status_data.status = Error;
  }

  return status_data.status;
}

void ISR() {
  enc1.tick();
  rewinder.tick();
  // packer_stepper.tick();
}

void setSettings() {
  
}

void isr() {
  turn++;
}

void setup() {
  Serial.begin(115200);
  lcd_manager.init();

  EEPROM.begin(512);
  // save();
  read();

  enc1.setType(TYPE2);

  Timer1.initialize(25);            // установка таймера на каждые 1000 микросекунд (= 1 мс)
  Timer1.attachInterrupt(ISR); 

  attachInterrupt(digitalPinToInterrupt(enc), isr, RISING);

  rewinder.setAcceleration(500);
  rewinder.setMaxSpeed(20000);

  setSettings();
}

void statusTask() {
  if(status_data.status == Work || status_data.status == Prepare && speed <= 0 && rewinder.getStatus() == 0) {
    status_data.task = 0;
    status_data.status = Done;
    ledcDetachPin(26);
  }
}

void loop() {
  encController();
  status();

  if (millis() - tmr0 > 50) {
    lcd_manager.tick();
    tmr0 = millis();
  } 

  if (millis() - tmr1 > 50) {
    if (status_data.task == 1) {
      // static int stat = 1;      // 0 - ожидание; 1 - разгон; 2 - равномерное движение; 3 - остановка

      if ((target - turn) * 25 <= (speed * speed) / (2 * settings_data.endAccelerate)) {
        speed -= settings_data.endAccelerate * 0.05;
        if (speed < 0) speed = 0;
      } else if (speed < settings_data.maxSpeed) {
        speed += settings_data.startAccelerate * 0.05;
        if (speed > settings_data.maxSpeed) speed = settings_data.maxSpeed;
      }
      ledcSetup(0, speed*4, 8);
    }

    statusTask();
    tmr1 = millis();
  }

  if (lcd_manager.settingChanged) {
      setSettings();                // обновляем настройки
      save();                       // сохраняем настройки
      Serial.println("сохранение...");
      lcd_manager.settingChanged = false;
    }

  if (millis() - tmr2 > 1000) {
    Serial.print(FPS);
    Serial.print("  ");
    Serial.print(status_decode[status_data.status]);
    Serial.print("  ");
    Serial.print(status_data.task);
    Serial.print("  ");
    Serial.print(target);
    Serial.print("  ");
    Serial.print(turn);
    Serial.print("  ");
    Serial.print(speed);
    Serial.print("  ");
    
    Serial.println("  ");
    tmr2 = millis();
    FPS = 0;
  }

  FPS++;
}