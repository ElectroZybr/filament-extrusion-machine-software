#include <Arduino.h>


class Micrometer {
public: 
    Micrometer(uint8_t dataPin, uint8_t clockPin);
    float GetValue();
    float GetInchValue();
    void begin();
    void calculation();
    void tick();
    volatile bool packageIsReady = false;
    bool error = true;
private:  
    uint8_t dataPin;
    uint8_t clockPin;
    static Micrometer* instance;
    void IRAM_ATTR readBit();
    
    volatile float value = 0;
    volatile long rawValue = 0;
    volatile int bitCounter = 0;
    volatile int sign = 1;
    volatile bool isInches = true;
    volatile uint32_t tmr = 0;

    static void IRAM_ATTR isrHandler(void* arg);
};