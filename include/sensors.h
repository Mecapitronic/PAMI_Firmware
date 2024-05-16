#ifndef SENSORS_H
#define SENSORS_H

#include "pin.h"
#include "ESP32_Helper.h"

#define OPPONENT_DISTANCE_MM 200
#define READ_TIME_PERIOD_MS 40

#define GP2Y0A41SK0F
#define COEF_A 41710//73762
#define COEF_B 13//57
#define COEF_C 300

// Déclaration des fonctions
void initSensor();
int readSensor();
bool checkOpponent(uint16_t distance = OPPONENT_DISTANCE_MM);

#endif
