#include "motion.h"
using namespace Printer;
using namespace std;

namespace Motion
{
  namespace
  {
    float NormalizeAngleDeg(float angleDeg)
    {
      float normalized = fmodf(angleDeg + 180.0f, 360.0f);
      if (normalized < 0.0f)
      {
        normalized += 360.0f;
      }
      return normalized - 180.0f;
    }

    // Position absolue du robot
    PoseF currentPose = {100.0f, 100.0f, 0.0f};
    // Déplacement cible polaire
    PolarMove targetMove = {0.0f, 0.0f, 0.0f};

    constexpr int stepMultiplier = StepMode::EIGHTH_STEP;
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
    motor_G.move(-stepValue);
    motor_D.move(stepValue);

    // Calculate absolute target position
    float headingRad = currentPose.h * (PI / 180.0f);
    Pose target = Pose((int16_t)(currentPose.x + _dist * cosf(headingRad)),
                       (int16_t)(currentPose.y + _dist * sinf(headingRad)),
                       (int16_t)currentPose.h);
    Screen::SetTarget(target);

    ProcessMove();
  }

  void Turn(float _angle)
  {
    long stepValue = ConvertAngleToStep(_angle);
    motor_G.move(-stepValue);
    motor_D.move(-stepValue);

    // Calculate absolute target orientation
    Pose target = Pose((int16_t)currentPose.x,
                       (int16_t)currentPose.y,
                       NormalizeAngleDeg(currentPose.h + _angle));
    Screen::SetTarget(target);

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
    currentPose.h = NormalizeAngleDeg(_rot);
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

      currentPose.x -= dCenter * cosf(headingMid);
      currentPose.y -= dCenter * sinf(headingMid);
      currentPose.h = NormalizeAngleDeg(currentPose.h + dThetaRad * (180.0f / PI));
      Screen::SetPose(Motion::GetCurrentPose());
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
        if (ToF_VL53L8CX::IsTargetPresent())
        {
          tempDistance_D = motor_D.distanceToGo();
          tempDistance_G = motor_G.distanceToGo();

          float speedTmp = GetMaxSpeed();
          float accelTmp = GetAcceleration();

          //SetAcceleration(stopAcceleration);
          //SetMaxSpeed(stopSpeed);

          //motor_D.move(0);
          //motor_G.move(0);
          motor_D.stopMove();
          motor_G.stopMove();

          //tempDistance_D = tempDistance_D + motor_D.distanceToGo();
          //tempDistance_G = tempDistance_G + motor_G.distanceToGo();

          println("Opponent detected");
          break;
          Timeout opponentTimeout;
          opponentTimeout.Start(2000);
          // break;
          // While we need to stop
          while (ToF_VL53L8CX::IsTargetPresent() && !opponentTimeout.IsTimeOut())
          {
            vTaskDelay(500);
            println("Opponent still here");
            updatePoseFromMotors(motor_D.currentPosition(), motor_G.currentPosition());
            tempDistance_D = motor_D.distanceToGo();
            tempDistance_G = motor_G.distanceToGo();
          }

          //SetAcceleration(accelTmp);
          //SetMaxSpeed(speedTmp);

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
    targetMove.rotation1 = NormalizeAngleDeg((targetAngleRadians - currentRotRadians) * (180.0f / M_PI));

    targetMove.rotation2 = 0; // Pas de rotation finale
    Pose target = Pose((int16_t)_x, (int16_t)_y, degrees(targetAngleRadians));
    Screen::SetTarget(target);
  }

  void ConvertToPolar(float _x, float _y, float _rot)
  {
    float dx = _x - currentPose.x;
    float dy = _y - currentPose.y;

    float targetAngleRadians = atan2(dy, dx);
    float currentRotRadians = currentPose.h * (M_PI / 180.0f);
    float targetRotRadians = _rot * (M_PI / 180.0f);

    targetMove.distance = sqrt(dx * dx + dy * dy);

    // Calculer la rotation la plus courte pour rotation1
    targetMove.rotation1 = NormalizeAngleDeg((targetAngleRadians - currentRotRadians) * (180.0f / M_PI));

    // Calculer la rotation la plus courte pour rotation2
    targetMove.rotation2 = NormalizeAngleDeg((targetRotRadians - targetAngleRadians) * (180.0f / M_PI));
    Pose target = Pose((int16_t)_x, (int16_t)_y, degrees(targetRotRadians));
    Screen::SetTarget(target);
  }

  void GoTo(PoseF _target)
  {
    GoTo(_target.x, _target.y, _target.h);
  }

  void GoTo(float _x, float _y)
  {
    ConvertToPolar(_x, _y);
    println("GoTo Polar : rot1=%f dist=%f rot2=%f", targetMove.rotation1, targetMove.distance, targetMove.rotation2);
    Turn(targetMove.rotation1);
    Go(targetMove.distance);
  }

  void GoTo(float _x, float _y, float _rot)
  {
    ConvertToPolar(_x, _y, _rot);
    println("GoTo Polar : rot1=%f dist=%f rot2=%f", targetMove.rotation1, targetMove.distance, targetMove.rotation2);
    Turn(targetMove.rotation1);
    Go(targetMove.distance);
    Turn(targetMove.rotation2);
  }

  void TurnTo(float _x, float _y)
  {
    ConvertToPolar(_x, _y);
    println("TurnTo Polar : rot1=%f dist=%f rot2=%f", targetMove.rotation1, targetMove.distance, targetMove.rotation2);
    Turn(targetMove.rotation1);
  }

} // namespace Motion