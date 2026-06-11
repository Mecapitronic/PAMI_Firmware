#ifndef PAMI_CONFIG_H
#define PAMI_CONFIG_H

#include <cstdint>

// Forward declaration
namespace IHM
{
  enum class Team;
}

namespace PamiConfiguration
{
  // x, y, waitMs
  struct PamiMovement
  {
    int x;
    int y;
    int waitMs;
  };

  /**
   * ServoId VL53
   * ServoId Bras
   * isNinja
   * startX, startY, startHeading
   * initialMove
   * matchMove1
   * matchMove2
   */
  struct PamiConfig
  {
    uint8_t servoIdVL53;
    uint8_t servoIdBras;
    bool isNinja;
    int startX;
    int startY;
    int startHeading;
    PamiMovement initialMove;
    PamiMovement matchMove1;
    PamiMovement matchMove2;
  };

  constexpr float fieldWidthMm = 3000.0f;

  float TeamAwareX(float x);
  float TeamAwareHeading(float heading);
  
  const PamiConfig* GetPamiConfig(int numPami);
  
  void ApplyStartPose(const PamiConfig& config);
  void ExecutePamiMovement(const PamiMovement& move);
  void ApplyAx12PamiConfig(const PamiConfig& config);
}

#endif // PAMI_CONFIG_H
