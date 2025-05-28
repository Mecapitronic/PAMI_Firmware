#ifndef MOTION_H
#define MOTION_H

#include <AccelStepper.h>
#include "pin.h"
#include "match.h"
#include "sensors.h"
#include "ESP32_Helper.h"

#define STEPS_PER_REVOLUTION 200    // Nombre de pas par tour du moteur
#define WHEEL_DIAMETER_MM 59.7f     // Diamètre de la roue en millimètres
#define WHEEL_DISTANCE_MM 87.0f     // Distance entre les roues en millimètres

#define MAX_SPEED       4800.0
#define MAX_ACCELERATION    3000.0

#define STOP_SPEED      48000.0
#define STOP_ACCELERATION   30000.0

#define CENTER_POSITION_MM 53       // Valeur entre l'arrière du robot et le centre des roues en millimètres

#define MOTION_WAIT 0
#define MOTION_RUN 1
#define MOTION_STOP 2

// Structure pour représenter un déplacement polaire relatif du robot
struct PolarMove {
    float rotation1; // Première rotation pour s'aligner vers la cible
    float distance;  // Déplacement linéaire vers la cible
    float rotation2; // Rotation finale pour ajuster l'orientation
};

enum StepMode {
    EIGHTH_STEP,      // MS1 = GND, MS2 = GND -> Current configuration
    HALF_STEP,        // MS1 = GND, MS2 = VIO
    QUARTER_STEP,     // MS1 = VIO, MS2 = GND
    SIXTEENTH_STEP    // MS1 = VIO, MS2 = VIO
};

// Déclaration des objets comme externes
extern AccelStepper motor_D;
extern AccelStepper motor_G;

// Déclaration des fonctions

void initMotion();
void enableMotors();
void disableMotors();
void setMaxSpeed(float _maxSpeed = MAX_SPEED);
float getMaxSpeed();
void setAcceleration(float _acceleration = MAX_ACCELERATION);
float getAcceleration();
void updateMotors();

PoseF getCurrentPose();

void setCurrentY(float _y);
void setCurrentX(float _x);
void setCurrentRot(float _rot);

void processMove();
void setOpponentChecking(bool _opponentChecking);
void setMotionState(int _motionState);

long convertDistToStep(float _dist);
long convertAngleToStep(float angle);

// Déplacements relatifs
void go(float _dist);
void turn(float _angle);
void turnGo(float _angle, float _dist);

// Déplacements absolus
void goTo(PoseF _target);
void goTo(float _x, float _y);
void goTo(float _x, float _y, float _rot);

void turnTo(float _x, float _y);

// Converti la position demandée vers le targetPolarMove
void convertToPolar(PoseF _target);
void convertToPolar(float _x, float _y);
void convertToPolar(float _x, float _y, float _rot);

#endif
