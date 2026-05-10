#include "Motion.h"
using namespace Printer;
using namespace std;

namespace Motion
{
  namespace
  {
    // Position absolue du robot
    PoseF currentPose = {0.0f, 0.0f, 0.0f};
    PoseF targetPose = {0.0f, 0.0f, 0.0f};
    // Déplacement cible polaire
    PolarMove targetMove = {0.0f, 0.0f, 0.0f};
    float tempTargetRotation = 0.0f;
    bool newPolarTarget = false;

    StepMode currentStepMode = EIGHTH_STEP;
    int stepMultiplier = 8;
    constexpr float circumferenceMM = wheelDiameterMm * PI; // 2 PI r

    // Variable Opponent
    bool opponentChecking = false;
  }
  // create the stepper motor object
  MotorDriver motor_G(PIN_STEP_M3, PIN_DIR_M3);
  MotorDriver motor_D(PIN_STEP_M2, PIN_DIR_M2);

  void Initialisation()
  {
    motor_D.begin();
    motor_G.begin();

    // Configure les vitesses et accelerations

    int speedPref = Preferences_Helper::LoadFromPreference("Speed",0);
    int accelPref = Preferences_Helper::LoadFromPreference("Accel",0);
    if(speedPref != 0)
    {
      Motion::SetMaxSpeed(speedPref);
      println("Speed Pref : %i", speedPref);
    }
    else
    {
      Motion::SetMaxSpeed(Motion::maxSpeed);
    }
    if(accelPref != 0)
    {
      Motion::SetAcceleration(accelPref);
      println("Accel Pref : %i", accelPref);
    }
    else
    {
      Motion::SetAcceleration(Motion::maxAcceleration);
    }
  }

  void SetMaxSpeed(float _maxSpeed)
  {
    motor_G.setMaxSpeed(_maxSpeed); // steps/s
    motor_D.setMaxSpeed(_maxSpeed); // steps/s
  }

  float GetMaxSpeed()
  {
    return motor_D.maxSpeed();
  }

  void SetAcceleration(float _acceleration)
  {
    motor_G.setAcceleration(_acceleration); // steps/s^2
    motor_D.setAcceleration(_acceleration); // steps/s^2
  }

  float GetAcceleration()
  {
    return motor_D.getAcceleration();
  }

  void Go(float _dist)
  {
    long stepValue = ConvertDistToStep(_dist);
    motor_G.move(stepValue);
    motor_D.move(-stepValue);
    ProcessMove();
  }

  void Turn(float _angle)
  {
    long stepValue = ConvertAngleToStep(_angle);
    motor_G.move(-stepValue);
    motor_D.move(-stepValue);
    ProcessMove();
  }

  PoseF GetCurrentPoseF()
  {
    return currentPose;
  }

  Pose GetCurrentPose()
  {
    return Pose((int16_t)currentPose.x, (int16_t)currentPose.y, (int16_t)currentPose.h);
  }

  void SetCurrentY(float _y)
  {
    currentPose.y = _y;
  }

  void SetCurrentX(float _x)
  {
    currentPose.x = _x;
  }

  void SetCurrentRot(float _rot)
  {
    currentPose.h = _rot;
  }

  void SetOpponentChecking(bool _opponentChecking)
  {
    opponentChecking = _opponentChecking;
  }

  void ProcessMove()
  {

    long tempDistance_D = 0;
    long tempDistance_G = 0;
    const float mmPerStep = circumferenceMM / (stepsPerRevolution * stepMultiplier);

    long prevPosD = motor_D.currentPosition();
    long prevPosG = motor_G.currentPosition();

    auto updatePoseFromMotors = [&](long posD, long posG)
    {
      const long deltaStepD = posD - prevPosD;
      const long deltaStepG = posG - prevPosG;
      prevPosD = posD;
      prevPosG = posG;

      if (deltaStepD == 0 && deltaStepG == 0)
      {
        return;
      }

      // Mapping des sens pour obtenir une cinematique coherente en mm.
      const float distGmm = deltaStepG * mmPerStep;
      const float distDmm = -deltaStepD * mmPerStep;

      const float dCenter = (distGmm + distDmm) * 0.5f;
      const float dThetaRad = (distDmm - distGmm) / wheelDistanceMm;

      const float headingRad = currentPose.h * (PI / 180.0f);
      const float headingMid = headingRad + dThetaRad * 0.5f;

      currentPose.x += dCenter * cosf(headingMid);
      currentPose.y += dCenter * sinf(headingMid);
      currentPose.h += dThetaRad * (180.0f / PI);
      Screen::SetPose(GetCurrentPose());
    };

    println("Processing Move...");
    while ((motor_D.isRunning() || motor_G.isRunning())) // && Match::matchState != Match::State::MATCH_END)
    {
      vTaskDelay(1);
      updatePoseFromMotors(motor_D.currentPosition(), motor_G.currentPosition());
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

          float speedTmp = GetMaxSpeed();
          float accelTmp = GetAcceleration();

          SetAcceleration(stopAcceleration);
          SetMaxSpeed(stopSpeed);

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

          SetAcceleration(accelTmp/4);
          SetMaxSpeed(speedTmp/4);

          motor_D.move(tempDistance_D);
          motor_G.move(tempDistance_G);
        }
      }
    }
    // Derniere mise a jour pour capter les derniers pas eventuels en sortie de boucle.
    updatePoseFromMotors(motor_D.currentPosition(), motor_G.currentPosition());
    println("Movement ok");
  }

  long ConvertDistToStep(float _dist)
  {
    float revolutions = _dist / circumferenceMM;
    return static_cast<long>(revolutions * stepsPerRevolution * stepMultiplier);
  }

  // Convert angle to step for one wheel (robot turning on center)
  long ConvertAngleToStep(float _angle)
  {
    float angleRadians = _angle * (PI / 180.0f);               // Conversion en radians
    float arcLength = (wheelDistanceMm / 2.0f) * angleRadians; // Longueur de l'arc parcouru par chaque roue
    float revolutions = arcLength / circumferenceMM;
    return static_cast<long>(revolutions * stepsPerRevolution * stepMultiplier);
  }

  void ConvertToPolar(PoseF _target)
  {
    ConvertToPolar(_target.x, _target.y, _target.h);
  }

  void ConvertToPolar(float _x, float _y)
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

  void ConvertToPolar(float _x, float _y, float _rot)
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

  void GoTo(PoseF _target)
  {
    GoTo(_target.x, _target.y, _target.h);
  }

  void GoTo(float _x, float _y)
  {
    ConvertToPolar(_x, _y);
    Turn(targetMove.rotation1);
    Go(targetMove.distance);
    newPolarTarget = false;
  }

  void GoTo(float _x, float _y, float _rot)
  {
    ConvertToPolar(_x, _y, _rot);
    Turn(targetMove.rotation1);
    Go(targetMove.distance);
    Turn(targetMove.rotation2);
    newPolarTarget = false;
  }

  void TurnTo(float _x, float _y)
  {
    ConvertToPolar(_x, _y);
    Turn(targetMove.rotation1);
    newPolarTarget = false;
  }

} // namespace Motion