#include "motion.h"
using namespace Printer;
using namespace std;

// Position absolue du robot
Pose currentPose = {0.0f, 0.0f, 0.0f};
Pose targetPose = {0.0f, 0.0f, 0.0f};
// Déplacement cible polaire
PolarMove targetMove = {0.0f, 0.0f, 0.0f};
float tempTargetRotation = 0.0f;
bool newPolarTarget = false;

int motionState = MOTION_WAIT;

StepMode currentStepMode = EIGHTH_STEP;
int stepMultiplier = 8;
float circumferenceMM = WHEEL_DIAMETER_MM * PI; // 2 PI r

// Variable Opponent
bool opponentChecking = false;

// Speed & Acceleration
float maxSpeed = 0;
float maxAccel = 0;

// create the stepper motor object
AccelStepper motor_G(AccelStepper::DRIVER, STEP_G, DIR_G);
AccelStepper motor_D(AccelStepper::DRIVER, STEP_D, DIR_D);

void initMotion()
{
  // Configure les pins
  pinMode(ENABLE_G, OUTPUT);
  pinMode(ENABLE_D, OUTPUT);
  pinMode(DIR_G, OUTPUT);
  pinMode(STEP_G, OUTPUT);
  pinMode(DIR_D, OUTPUT);
  pinMode(STEP_D, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);

  // Configure les pas
  setStepMode(EIGHTH_STEP);
  // Désactive les moteurs
  digitalWrite(ENABLE_G, HIGH);
  digitalWrite(ENABLE_D, HIGH);
  // Configure les vitesses et accelerations
  setMaxSpeed();
  setAcceleration();
  // Motion est ready
  setMotionState(MOTION_RUN);
}

void setMaxSpeed(float _maxSpeed)
{
    motor_G.setMaxSpeed(_maxSpeed); // steps/s
    motor_D.setMaxSpeed(_maxSpeed); // steps/s
  maxSpeed = _maxSpeed;
}

float getMaxSpeed()
{
  return maxSpeed;
}

void setAcceleration(float _acceleration)
{
    motor_G.setAcceleration(_acceleration); // steps/s^2
    motor_D.setAcceleration(_acceleration); // steps/s^2
  maxAccel = _acceleration;
}

float getAcceleration()
{
  return maxAccel;
}

void setStepMode(StepMode mode)
{
  currentStepMode = mode;
  switch (mode)
  {
  case EIGHTH_STEP:
    digitalWrite(MS1, LOW);
    digitalWrite(MS2, LOW);
    stepMultiplier = 8;
    break;
  case HALF_STEP:
    digitalWrite(MS1, HIGH);
    digitalWrite(MS2, LOW);
    stepMultiplier = 2;
    break;
  case QUARTER_STEP:
    digitalWrite(MS1, LOW);
    digitalWrite(MS2, HIGH);
    stepMultiplier = 4;
    break;
  case SIXTEENTH_STEP:
    digitalWrite(MS1, HIGH);
    digitalWrite(MS2, HIGH);
    stepMultiplier = 16;
    break;
  }
}

void enableMotors()
{
  digitalWrite(ENABLE_G, LOW);
  digitalWrite(ENABLE_D, LOW);
}

void disableMotors()
{
  digitalWrite(ENABLE_G, HIGH);
  digitalWrite(ENABLE_D, HIGH);
}

void go(float _dist)
{
  long stepValue = convertDistToStep(_dist);
    motor_G.move(-stepValue);
    motor_D.move(stepValue);
  processMove();
}

void turn(float _angle)
{
  long stepValue = convertAngleToStep(_angle);
  int pami = ESP32_Helper::GetNumPami();
    motor_G.move(stepValue);
    motor_D.move(stepValue);
  processMove();
}

void updateMotors()
{
  motor_D.run();
  motor_G.run();
}

Pose getCurrentPose()
{
  return currentPose;
}

void setCurrentY(float _y)
{
  currentPose.setY(_y);
}

void setCurrentX(float _x)
{
  currentPose.setX(_x);
}

void setCurrentRot(float _rot)
{
  currentPose.setRot(_rot);
}

void setOpponentChecking(bool _opponentChecking)
{
  opponentChecking = _opponentChecking;
}

void setMotionState(int _motionState)
{
  motionState = _motionState;
}

void processMove()
{

  long tempDistance_D = 0;
  long tempDistance_G = 0;

  println("Processing Move...");

  while ((motor_D.isRunning() || motor_G.isRunning()) && getMatchState() != MATCH_END)
  {
    vTaskDelay(1);
    // Serial.print(">Dspeed:");Serial.println(motor_D.speed());
    // Serial.print(">DdistanceToGo:");Serial.println( (int)motor_D.distanceToGo());
    // Serial.print(">Gspeed:");Serial.println( motor_G.speed());
    // Serial.print(">GdistanceToGo:");Serial.println( (int)motor_G.distanceToGo());
    if (opponentChecking)
    {
      if (checkOpponent())
      {
        println("Opponent detected");
        tempDistance_D = motor_D.distanceToGo();
        tempDistance_G = motor_G.distanceToGo();

        float speedTmp = maxSpeed;
        float accelTmp = maxAccel;

        setAcceleration(STOP_ACCELERATION);
        setMaxSpeed(STOP_SPEED);

        motor_D.move(0);
        motor_G.move(0);

        tempDistance_D = tempDistance_D + motor_D.distanceToGo();
        tempDistance_G = tempDistance_G + motor_G.distanceToGo();

        while (checkOpponent())
        {
          vTaskDelay(100 / portTICK_PERIOD_MS);
          println("Opponent still here");
        }

        setAcceleration(accelTmp);
        setMaxSpeed(speedTmp);

        motor_D.move(tempDistance_D);
        motor_G.move(tempDistance_G);
      }
    }
  }
  println("Movement ok");
}

long convertDistToStep(float _dist)
{
  float revolutions = _dist / circumferenceMM;
  return static_cast<long>(revolutions * STEPS_PER_REVOLUTION * stepMultiplier);
}

// Convert angle to step for one wheel (robot turning on center)
long convertAngleToStep(float _angle)
{
  float angleRadians = _angle * (PI / 180.0f);                 // Conversion en radians
  float arcLength = (WHEEL_DISTANCE_MM / 2.0f) * angleRadians; // Longueur de l'arc parcouru par chaque roue
  float revolutions = arcLength / circumferenceMM;
  return static_cast<long>(revolutions * STEPS_PER_REVOLUTION * stepMultiplier);
}

void convertToPolar(Pose _target)
{
  convertToPolar(_target.x, _target.y, _target.rot);
}

void convertToPolar(float _x, float _y)
{
  float dx = _x - currentPose.x;
  float dy = _y - currentPose.y;

  float targetAngleRadians = atan2(dy, dx);
  float currentRotRadians = currentPose.rot * (M_PI / 180.0f);

  targetMove.distance = sqrt(dx * dx + dy * dy);

  // Calculer la rotation la plus courte pour rotation1
  targetMove.rotation1 = (targetAngleRadians - currentRotRadians) * (180.0f / M_PI);
  if (targetMove.rotation1 > 180.0f)
    targetMove.rotation1 -= 360.0f;
  if (targetMove.rotation1 < -180.0f)
    targetMove.rotation1 += 360.0f;

  tempTargetRotation = targetAngleRadians * (180.0f / M_PI);

  targetMove.rotation2 = 0; // Pas de rotation finale

  newPolarTarget = true;
}

void convertToPolar(float _x, float _y, float _rot)
{
  float dx = _x - currentPose.x;
  float dy = -(_y - currentPose.y);

  float targetAngleRadians = atan2(dy, dx);
  float currentRotRadians = currentPose.rot * (M_PI / 180.0f);
  float targetRotRadians = _rot * (M_PI / 180.0f);

  targetMove.distance = sqrt(dx * dx + dy * dy);

  // Calculer la rotation la plus courte pour rotation1
  targetMove.rotation1 = (targetAngleRadians - currentRotRadians) * (180.0f / M_PI);
  if (targetMove.rotation1 > 180.0f)
    targetMove.rotation1 -= 360.0f;
  if (targetMove.rotation1 < -180.0f)
    targetMove.rotation1 += 360.0f;

  // Calculer la rotation la plus courte pour rotation2
  targetMove.rotation2 = (targetRotRadians - targetAngleRadians) * (180.0f / M_PI);
  if (targetMove.rotation2 > 180.0f)
    targetMove.rotation2 -= 360.0f;
  if (targetMove.rotation2 < -180.0f)
    targetMove.rotation2 += 360.0f;

  newPolarTarget = true;
}

void goTo(Pose _target)
{
  goTo(_target.x, _target.y, _target.rot);
}

void goTo(float _x, float _y)
{
  convertToPolar(_x, _y);
  int pami = ESP32_Helper::GetNumPami();
  if (pami == 2 || pami == 1)
  {
    targetMove.rotation1 = targetMove.rotation1 * 2;
    targetMove.distance = targetMove.distance * 2;
  }
  turn(targetMove.rotation1);
  go(targetMove.distance);
  currentPose.setX(_x);
  currentPose.setY(_y);
  currentPose.setRot(tempTargetRotation);
  newPolarTarget = false;
}

void goTo(float _x, float _y, float _rot)
{
  convertToPolar(_x, _y, _rot);
  int pami = ESP32_Helper::GetNumPami();
  if (pami == 2 || pami == 1)
  {
    targetMove.rotation1 = targetMove.rotation1 * 2;
    targetMove.distance = targetMove.distance * 2;
    targetMove.rotation2 = targetMove.rotation2 * 2;
  }
  turn(targetMove.rotation1);
  go(targetMove.distance);
  currentPose.setX(_x);
  currentPose.setY(_y);
  turn(targetMove.rotation2);
  currentPose.setRot(_rot);
  newPolarTarget = false;
}

void turnTo(float _x, float _y)
{
  convertToPolar(_x, _y);
  int pami = ESP32_Helper::GetNumPami();
  if (pami == 2 || pami == 1)
  {
    targetMove.rotation1 = targetMove.rotation1 * 2;
    targetMove.distance = targetMove.distance * 2;
  }
  turn(targetMove.rotation1);
  currentPose.setRot(tempTargetRotation);
  newPolarTarget = false;
}