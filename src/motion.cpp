#include "motion.h"
using namespace Printer;
using namespace std;

// Position absolue du robot
PoseF currentPose = {0.0f, 0.0f, 0.0f};
PoseF targetPose = {0.0f, 0.0f, 0.0f};
// Déplacement cible polaire
PolarMove targetMove = {0.0f, 0.0f, 0.0f};
float tempTargetRotation = 0.0f;
bool newPolarTarget = false;

int motionState = MOTION_WAIT;

StepMode currentStepMode = EIGHTH_STEP;
int stepMultiplier = 8;
constexpr float circumferenceMM = WHEEL_DIAMETER_MM * PI; // 2 PI r

// Variable Opponent
bool opponentChecking = false;

// Speed & Acceleration
float maxSpeed = 0;
float maxAccel = 0;

// create the stepper motor object
AccelStepper motor_G(AccelStepper::DRIVER, PIN_STEP_M3, PIN_DIR_M3);
AccelStepper motor_D(AccelStepper::DRIVER, PIN_STEP_M2, PIN_DIR_M2);

void initMotion()
{
  // Configure les pins
  pinMode(PIN_EN_MCU, OUTPUT);
  pinMode(PIN_DIR_M2, OUTPUT);
  pinMode(PIN_STEP_M2, OUTPUT);
  pinMode(PIN_DIR_M3, OUTPUT);
  pinMode(PIN_STEP_M3, OUTPUT);

  // Désactive les moteurs
  digitalWrite(PIN_EN_MCU, LOW);
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

void enableMotors()
{
  digitalWrite(PIN_EN_MCU, HIGH);
}

void disableMotors()
{
  digitalWrite(PIN_EN_MCU, LOW);
}

void go(float _dist)
{
  long stepValue = convertDistToStep(_dist);
  motor_G.move(stepValue);
  motor_D.move(-stepValue);
  processMove();
}

void turn(float _angle)
{
  long stepValue = convertAngleToStep(_angle);
  motor_G.move(-stepValue);
  motor_D.move(-stepValue);
  processMove();
}

void updateMotors()
{
  motor_D.run();
  motor_G.run();
}

PoseF getCurrentPose()
{
  return currentPose;
}

void setCurrentY(float _y)
{
  currentPose.y = _y;
}

void setCurrentX(float _x)
{
  currentPose.x = _x;
}

void setCurrentRot(float _rot)
{
  currentPose.h = _rot;
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

  while ((motor_D.isRunning() || motor_G.isRunning()) && matchState != State::MATCH_END)
  {
    vTaskDelay(1);
    // print(">Dspeed:");println(motor_D.speed());
    // print(">DdistanceToGo:");println( (int)motor_D.distanceToGo());
    // print(">Gspeed:");println( motor_G.speed());
    // print(">GdistanceToGo:");println( (int)motor_G.distanceToGo());
    if (opponentChecking)
    {
      // if we need to stop
      if (false)
      {
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

        println("Opponent detected");
        // While we need to stop
        while (false)
        {
          vTaskDelay(1000 / portTICK_PERIOD_MS);
          println("Opponent still here");
        }

        setAcceleration(accelTmp/4);
        setMaxSpeed(speedTmp/4);

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

void convertToPolar(PoseF _target)
{
  convertToPolar(_target.x, _target.y, _target.h);
}

void convertToPolar(float _x, float _y)
{
  float dx = _x - currentPose.x;
  float dy = _y - currentPose.y;

  float targetAngleRadians = atan2(dy, dx);
  float currentRotRadians = currentPose.h * (M_PI / 180.0f);

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
  float currentRotRadians = currentPose.h * (M_PI / 180.0f);
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

void goTo(PoseF _target)
{
  goTo(_target.x, _target.y, _target.h);
}

void goTo(float _x, float _y)
{
  convertToPolar(_x, _y);
  turn(targetMove.rotation1);
  go(targetMove.distance);
  currentPose.x = (_x);
  currentPose.y = (_y);
  currentPose.h = (tempTargetRotation);
  newPolarTarget = false;
}

void goTo(float _x, float _y, float _rot)
{
  convertToPolar(_x, _y, _rot);
  turn(targetMove.rotation1);
  go(targetMove.distance);
  currentPose.x = (_x);
  currentPose.y = (_y);
  turn(targetMove.rotation2);
  currentPose.h = (_rot);
  newPolarTarget = false;
}

void turnTo(float _x, float _y)
{
  convertToPolar(_x, _y);
  turn(targetMove.rotation1);
  currentPose.h = (tempTargetRotation);
  newPolarTarget = false;
}