#include <DigitalMicrometer.h>


Micrometer::Micrometer(uint8_t dataPin, uint8_t clockPin) :
    dataPin(dataPin), clockPin(clockPin) {}

void IRAM_ATTR Micrometer::isrHandler(void* arg) {
    Micrometer* instance = static_cast<Micrometer*>(arg);
    instance->readBit();
}

void IRAM_ATTR Micrometer::readBit() { 
    uint32_t now = xTaskGetTickCountFromISR() * portTICK_PERIOD_MS;

    if (now - tmr > 10) {   // если с момента последнего вызова прошло 10 мс начинаем принимать новый пакет
        rawValue = 0;
        bitCounter = 0;
        sign = 1;
        isInches = true;
    }
    tmr = now;

    bool bitValue = digitalRead(dataPin);

    if (bitCounter <= 24) {
        if (bitValue==HIGH) {
            if (bitCounter < 20) {
                rawValue |= 1 << bitCounter;
            }
            if (bitCounter == 20) {
                sign=-1;
            }
            if (bitCounter == 23) {
                isInches = false;
            }
        }
        bitCounter++;
    }
      
    if (bitCounter >= 24) {
        packageIsReady = true;
    }
}

void Micrometer::calculation() {
    if (packageIsReady) {
        int32_t tempValue = rawValue * sign;

        if (isInches) {
            tempValue = tempValue * (2.54 / 2); // перевод дюймов в милиметры
        }
        value = float(rawValue * sign) / 1000;

        packageIsReady = false;
        error = false;
    }
}

void Micrometer::begin() {
    pinMode(dataPin, INPUT_PULLUP);
    pinMode(clockPin, INPUT_PULLUP);
    attachInterruptArg(digitalPinToInterrupt(clockPin), isrHandler, this, RISING);
}

void Micrometer::tick() {
    if (xTaskGetTickCountFromISR() * portTICK_PERIOD_MS - tmr > 500) error = true;
    else if (packageIsReady) calculation();
}

float Micrometer::GetValue() {
    return value;
}

float Micrometer::GetInchValue() {
    return value * 2.54f;
}