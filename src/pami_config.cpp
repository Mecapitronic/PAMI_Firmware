#include "pami_config.h"
#include "main.h"
#include "Hardware_Config.h"
#include <Motion.h>
#include <ServoAX12.h>
#include <Printer.h>
#include <IHM.h>

using namespace Printer;
using namespace Hardware_Config;
using namespace ServoAX12;

namespace PamiConfiguration
{
  static const PamiConfig pamiConfigs[] = {
    {1, 2, false, 81, 1912, -90, {81, 1650, 0}, {81, 950, 0}, {0, 0, 0}},
    {3, 17, false, 213, 1912, -90, {213, 1650, 0}, {213, 1200, 0}, {800, 950, 0}},
    {1, 2, false, 387, 1912, -90, {387, 1650, 0}, {387, 1320, 0}, {1400, 900, 0}},
    {1, 2, false, 519, 1912, -90, {519, 1650, 0}, {519, 1450, 0}, {1130, 1450, 0}},
    {1, 2, false, 750, 1900, -90, {750, 1810, 0}, {750, 1800, 0}, {0, 0, 0}},
  };
  constexpr int pamiConfigCount = sizeof(pamiConfigs)/sizeof(PamiConfig);

  float TeamAwareX(float x)
  {
    return (IHM::team == IHM::Team::Bleu) ? (fieldWidthMm - x) : x;
  }

  float TeamAwareHeading(float heading)
  {
    float teamAwareHeading = (IHM::team == IHM::Team::Bleu) ? (180.0f - heading) : heading;
    while (teamAwareHeading > 180.0f)
    {
      teamAwareHeading -= 360.0f;
    }
    while (teamAwareHeading < -180.0f)
    {
      teamAwareHeading += 360.0f;
    }
    return teamAwareHeading;
  }

  const PamiConfig* GetPamiConfig(int numPami)
  {
    if (numPami < 1 || numPami > pamiConfigCount)
    {
      return nullptr;
    }
    return &pamiConfigs[numPami - 1];
  }

  void ApplyStartPose(const PamiConfig& config)
  {
    Motion::SetCurrentRot(TeamAwareHeading(config.startHeading));
    Motion::SetCurrentX(TeamAwareX(config.startX));
    Motion::SetCurrentY(config.startY);
  }

  void ExecutePamiMovement(const PamiMovement& move)
  {
    if (move.x == 0 || move.y == 0)
    {
      // No movement
      return;
    }
    Motion::GoTo(TeamAwareX(move.x), move.y);
    if (move.waitMs > 0)
    {
      delay(move.waitMs);
    }
  }

  void ApplyAx12PamiConfig(const PamiConfig& config)
  {
    const ServoConfig vl53Servo(config.servoIdVL53,
                                std::array<int32_t, MAX_SERVO_POSITIONS>{0, 160, 200, 160, 360},
                                5);
    const ServoConfig brasServo(config.servoIdBras,
                                std::array<int32_t, MAX_SERVO_POSITIONS>{0, 160, 250, 160, 360},
                                5);

    AddServo(ServoID::VL53, "VL53", vl53Servo);
    AddServo(ServoID::Bras, "Bras", brasServo);
  }
}
