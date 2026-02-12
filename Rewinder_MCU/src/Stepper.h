#include <Arduino.h>

class Stepper {
    private:
        uint16_t _steps;
        byte _stepPin;
        byte _dirPin;
        uint16_t _speed;
        uint16_t _stepTime;
        uint16_t _highTime = 4;
        uint32_t _tmr;
    public:
        Stepper (uint16_t steps, uint8_t step, uint8_t dir) {
            _steps = steps;
            _stepPin = step;
            _dirPin = dir;
            pinMode(_stepPin, OUTPUT);
            _highTime = 40;
            _tmr = micros();
        }

        void setSpeed(uint16_t speed) {
            _speed = speed;
            _stepTime = 1000000 / _speed * 2;
        }

        void tick() {
            uint32_t time = micros();
            if (time - _tmr >= _highTime) {
                digitalWrite(_stepPin, LOW);
            }
            if (time - _tmr >= _stepTime) {
                digitalWrite(_stepPin, HIGH);
                _tmr = time;
            } 
        }
};