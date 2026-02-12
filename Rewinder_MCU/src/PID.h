#include <Arduino.h>

class PID {
    private:
        float kp;
        float ki;
        float kd;
        uint32_t dt;
        int minOut;
        int maxOut;

        float integral = 0;
        float prevErr = 0;
    public: 
        float setpoint;
    public:
        PID (float new_kp, float new_ki, float new_kd, uint32_t new_dt = 100) {
            kp = new_kp;
            ki = new_ki;
            kd = new_kd;
            dt = new_dt;
        }

        void setLimits(int min, int max) {
            minOut = min;
            maxOut = max;
        }

        void reset() {
            integral = 0;
            prevErr = 0;
        }

        int computePID(float input) {
            float err = setpoint - input;
            integral = constrain(integral + (float)err * dt * ki, minOut, maxOut);
            float D = (err - prevErr) / dt;
            prevErr = err;
            return constrain(err * kp + integral + D * kd, minOut, maxOut);
        }
};