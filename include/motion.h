#ifndef MOTION_H
#define MOTION_H

#include "MotorDriver.h"
#include "pins.h"
#include "sensors.h"
#include "ESP32_Helper.h"
#include "ScreenSSD1306.h"

namespace Motion {

constexpr int stepsPerRevolution = 200;       // Nombre de pas par tour du moteur
constexpr float wheelDiameterMm = 59.6f;      // Diamètre de la roue en millimètres
constexpr float wheelDistanceMm = 88.8f;      // Distance entre les roues en millimètres

constexpr float maxSpeed = 4700.0f;
constexpr float maxAcceleration = 5000.0f;

constexpr float stopSpeed = 48000.0f;
constexpr float stopAcceleration = 30000.0f;

constexpr int centerPositionMm = 53;          // Valeur entre l'arrière du robot et le centre des roues en millimètres

// Structure pour représenter un déplacement polaire relatif du robot
struct PolarMove {
    float rotation1; // Première rotation pour s'aligner vers la cible
    float distance;  // Déplacement linéaire vers la cible
    float rotation2; // Rotation finale pour ajuster l'orientation
};

enum StepMode {
    EIGHTH_STEP = 8,      // MS1 = GND, MS2 = GND -> Current configuration
    HALF_STEP = 2,        // MS1 = GND, MS2 = VIO
    QUARTER_STEP = 4,     // MS1 = VIO, MS2 = GND
    SIXTEENTH_STEP = 16   // MS1 = VIO, MS2 = VIO
};

// Déclaration des fonctions

void Initialisation();
void SetMaxSpeed(float _maxSpeed = maxSpeed);
float GetMaxSpeed();
void SetAcceleration(float _acceleration = maxAcceleration);
float GetAcceleration();

PoseF GetCurrentPoseF();
Pose GetCurrentPose();

void SetCurrentY(float _y);
void SetCurrentX(float _x);
void SetCurrentRot(float _rot);

void ProcessMove();
void SetOpponentChecking(bool _opponentChecking);

long ConvertDistToStep(float _dist);
long ConvertAngleToStep(float angle);

// Déplacements relatifs
void Go(float _dist);
void Turn(float _angle);
void TurnGo(float _angle, float _dist);

// Déplacements absolus
void GoTo(PoseF _target);
void GoTo(float _x, float _y);
void GoTo(float _x, float _y, float _rot);

void TurnTo(float _x, float _y);

// Converti la position demandée vers le targetPolarMove
void ConvertToPolar(PoseF _target);
void ConvertToPolar(float _x, float _y);
void ConvertToPolar(float _x, float _y, float _rot);

// Déclaration des objets moteurs comme externes
extern MotorDriver motor_D;
extern MotorDriver motor_G;

} // namespace Motion

#endif
