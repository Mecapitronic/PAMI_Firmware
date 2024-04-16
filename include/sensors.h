#ifndef SENSORS_H
#define SENSORS_H

#include "pin.h"

#define OPPONENT_DISTANCE_MM 200
#define READ_TIME_PERIOD_MS 40

// Déclaration des fonctions
void initSensor();
bool readSensors(bool setDebug = false);
bool checkOpponent(uint16_t distance = OPPONENT_DISTANCE_MM);

#endif // SENSORS_H
