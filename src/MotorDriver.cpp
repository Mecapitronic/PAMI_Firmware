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
        Printer::printError("Failed to initialize stepper on pin " + String(stepPin));
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
    if (simulation)
    {
        simConfigureProfile(relative);
        return;
    }
    stepper ? stepper->move(relative) : false;
}

bool MotorDriver::isRunning() const
{
    if (simulation)
    {
        simAdvancePosition();
        return simCurrentPos != simTargetPos;
    }
    return stepper ? stepper->isRunning() : false;
}

long MotorDriver::distanceToGo() const
{
    if (simulation)
    {
        return simTargetPos - simCurrentPos;
    }
    return stepper ? stepper->targetPos() - stepper->getCurrentPosition() : 0;
}

long MotorDriver::targetPosition() const
{
    if (simulation)
    {
        return simTargetPos;
    }
    return stepper ? stepper->targetPos() : 0;
}

long MotorDriver::currentPosition() const
{
    if (simulation)
    {
        simAdvancePosition();
        return simCurrentPos;
    }
    return stepper ? stepper->getCurrentPosition() : 0;
}

float MotorDriver::speed() const
{
    if (simulation)
    {
        return simSpeedNow();
    }
    return stepper ? (float)stepper->getCurrentSpeedInMilliHz() / 1000.0f : 0.0f;
}

void MotorDriver::stopMove()
{
    if (simulation)
    {
        simStop();
        return;
    }
    if (stepper)
    {
        stepper->forceStop();
    }
}

#ifdef MOTOR_DRIVER_H

unsigned long MotorDriver::computeMoveDurationMs(long steps) const
{
    if (steps == 0 || simPeakSpeedStepsS <= 0.0f || simAccelStepsS2 <= 0.0f)
    {
        return 0;
    }
    const float durationS = (2.0f * simAccelTimeS) + simCruiseTimeS;
    const float durationMs = durationS * 1000.0f;
    return (unsigned long)max(1.0f, durationMs);
}

void MotorDriver::simConfigureProfile(long relative)
{
    simAdvancePosition();

    simStartPos = simCurrentPos;
    simTargetPos = simCurrentPos + relative;
    simTotalSteps = labs(relative);
    simDirection = (relative > 0) ? 1 : ((relative < 0) ? -1 : 0);

    const float configuredV = max(1.0f, maxSpeedCache);
    simAccelStepsS2 = max(1.0f, (accelerationCache > 0.0f) ? accelerationCache : configuredV);

    if (simTotalSteps == 0)
    {
        simResetProfile();
        simMoveStartMs = millis();
        return;
    }

    const float fullAccelTimeS = configuredV / simAccelStepsS2;
    const float fullAccelSteps = 0.5f * simAccelStepsS2 * fullAccelTimeS * fullAccelTimeS;

    if ((float)simTotalSteps <= 2.0f * fullAccelSteps)
    {
        // Triangular profile
        simAccelTimeS = sqrtf((float)simTotalSteps / simAccelStepsS2);
        simCruiseTimeS = 0.0f;
        simPeakSpeedStepsS = simAccelStepsS2 * simAccelTimeS;
    }
    else
    {
        // Trapezoidal profile
        simAccelTimeS = fullAccelTimeS;
        simPeakSpeedStepsS = configuredV;
        const float accelAndDecelSteps = 2.0f * fullAccelSteps;
        const float cruiseSteps = (float)simTotalSteps - accelAndDecelSteps;
        simCruiseTimeS = cruiseSteps / simPeakSpeedStepsS;
    }

    simMoveStartMs = millis();
    simMoveDurationMs = computeMoveDurationMs(simTotalSteps);
}

void MotorDriver::simResetProfile()
{
    simTotalSteps = 0;
    simDirection = 0;
    simAccelStepsS2 = 0.0f;
    simPeakSpeedStepsS = 0.0f;
    simAccelTimeS = 0.0f;
    simCruiseTimeS = 0.0f;
    simMoveDurationMs = 0;
}

float MotorDriver::simSpeedNow() const
{
    if (simCurrentPos == simTargetPos || simMoveDurationMs == 0)
    {
        return 0.0f;
    }

    const unsigned long elapsedMs = millis() - simMoveStartMs;
    if (elapsedMs >= simMoveDurationMs)
    {
        return 0.0f;
    }

    const float t = (float)elapsedMs / 1000.0f;
    return simSpeedAtTimeS(t);
}

float MotorDriver::simSpeedAtTimeS(float t) const
{
    const float t1 = simAccelTimeS;
    const float t2 = t1 + simCruiseTimeS;
    const float a = simAccelStepsS2;

    float v = 0.0f;
    if (t <= t1)
    {
        v = a * t;
    }
    else if (t <= t2)
    {
        v = simPeakSpeedStepsS;
    }
    else
    {
        const float tDec = t - t2;
        v = simPeakSpeedStepsS - a * tDec;
    }

    return max(0.0f, v);
}

float MotorDriver::simTraveledAtTimeS(float t) const
{
    const float t1 = simAccelTimeS;
    const float t2 = t1 + simCruiseTimeS;
    const float a = simAccelStepsS2;
    const float v = simPeakSpeedStepsS;

    const float accelSteps = 0.5f * a * t1 * t1;
    const float cruiseSteps = v * simCruiseTimeS;

    float traveled = 0.0f;
    if (t <= t1)
    {
        traveled = 0.5f * a * t * t;
    }
    else if (t <= t2)
    {
        traveled = accelSteps + v * (t - t1);
    }
    else
    {
        const float tDec = t - t2;
        traveled = accelSteps + cruiseSteps + v * tDec - 0.5f * a * tDec * tDec;
    }

    if (traveled < 0.0f)
    {
        return 0.0f;
    }
    return min((float)simTotalSteps, traveled);
}

void MotorDriver::simStop()
{
    simAdvancePosition();  // Freeze at current interpolated position
    simTargetPos = simCurrentPos;
    simStartPos = simCurrentPos;
    simResetProfile();
}

void MotorDriver::simAdvancePosition() const
{
    if (simCurrentPos == simTargetPos)
    {
        return;
    }
    if (simMoveDurationMs == 0)
    {
        simCurrentPos = simTargetPos;
        return;
    }
    const unsigned long elapsed = millis() - simMoveStartMs;
    if (elapsed >= simMoveDurationMs)
    {
        simCurrentPos = simTargetPos;
        return;
    }

    const float t = (float)elapsed / 1000.0f;
    const float traveled = simTraveledAtTimeS(t);

    long pos = simStartPos + (long)(traveled * (float)simDirection);
    if (simDirection > 0 && pos > simTargetPos)
    {
        pos = simTargetPos;
    }
    else if (simDirection < 0 && pos < simTargetPos)
    {
        pos = simTargetPos;
    }
    simCurrentPos = pos;
}
#endif