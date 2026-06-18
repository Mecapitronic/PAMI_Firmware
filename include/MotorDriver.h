#ifndef MOTOR_DRIVER_H
#define MOTOR_DRIVER_H

#include <Arduino.h>
#include <FastAccelStepper.h>
#include "ESP32_Hardware.h"

class MotorDriver
{
public:
    MotorDriver(uint8_t stepPin, uint8_t dirPin);
    ~MotorDriver() = default;

    void begin(uint8_t cpu_core = 1);

    void setMaxSpeed(float speed);
    float maxSpeed() const;

    void setAcceleration(float acceleration);
    float getAcceleration() const;

    void move(long relative);
    bool isRunning() const;

    long distanceToGo() const;
    long targetPosition() const;
    long currentPosition() const;

    float speed() const;
    void stopMove();

private:
    uint8_t stepPin = 0;
    uint8_t dirPin = 0;
    FastAccelStepper *stepper = nullptr;

    // We need only one instance of the engine, so we can make it static and initialize it only once
    static FastAccelStepperEngine engine;
    static bool engineInitialized;

    float maxSpeedCache = 0.0f;
    float accelerationCache = 0.0f;

    // Simulation of motor position without hardware RMT/MCPWM peripheral
    mutable long simCurrentPos = 0;
    long simTargetPos = 0;
    long simStartPos = 0;
    long simTotalSteps = 0;
    int8_t simDirection = 0;
    float simAccelStepsS2 = 0.0f;
    float simPeakSpeedStepsS = 0.0f;
    float simAccelTimeS = 0.0f;
    float simCruiseTimeS = 0.0f;
    mutable unsigned long simMoveStartMs = 0;
    mutable unsigned long simMoveDurationMs = 0;

    void simConfigureProfile(long relative);
    void simResetProfile();
    float simSpeedNow() const;
    float simSpeedAtTimeS(float t) const;
    float simTraveledAtTimeS(float t) const;
    void simStop();

    unsigned long computeMoveDurationMs(long steps) const;
    void simAdvancePosition() const;
};

#endif
