#include "MotorDriver.h"

FastAccelStepperEngine MotorDriver::engine;
bool MotorDriver::engineInitialized = false;

MotorDriver::MotorDriver(uint8_t stepPin, uint8_t dirPin)
    : stepPin(stepPin), dirPin(dirPin)
{
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
}

void MotorDriver::begin(uint8_t cpu_core)
{
    if (!engineInitialized)
    {
        engine.init(cpu_core);
        engineInitialized = true;
    }

    stepper = engine.stepperConnectToPin(stepPin);
    if (!stepper)
    {
        return;
    }

    stepper->setDirectionPin(dirPin);
    stepper->setAutoEnable(false);
}

void MotorDriver::setMaxSpeed(float speed)
{
    maxSpeedCache = speed;
    if (stepper)
    {
        stepper->setSpeedInHz((uint32_t)max(1.0f, speed));
        stepper->applySpeedAcceleration();
    }
}

float MotorDriver::maxSpeed() const
{
    return maxSpeedCache;
}

void MotorDriver::setAcceleration(float acceleration)
{
    accelerationCache = acceleration;
    if (stepper)
    {
        stepper->setAcceleration((int32_t)max(1.0f, acceleration));
        stepper->applySpeedAcceleration();
    }
}

float MotorDriver::getAcceleration() const
{
    return accelerationCache;
}

void MotorDriver::move(long relative)
{
    if (!stepper)
    {
        return;
    }
    stepper->move(relative);
}

bool MotorDriver::isRunning() const
{
    return stepper ? stepper->isRunning() : false;
}

long MotorDriver::distanceToGo() const
{
    if (!stepper)
    {
        return 0;
    }
    return stepper->targetPos() - stepper->getCurrentPosition();
}

long MotorDriver::targetPosition() const
{
    return stepper ? stepper->targetPos() : 0;
}

long MotorDriver::currentPosition() const
{
    return stepper ? stepper->getCurrentPosition() : 0;
}

float MotorDriver::speed() const
{
    return stepper ? (float)stepper->getCurrentSpeedInMilliHz() / 1000.0f : 0.0f;
}
