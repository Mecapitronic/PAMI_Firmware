#ifndef MAIN_H
#define MAIN_H

#include "ESP32_Helper.h"

using namespace Printer;
using namespace std;

#include "pin.h"
#include "motion.h"

TeamColor teamColor = TEAM_NONE;
Enable tirette = ENABLE_NONE;

TaskHandle_t Task1;
TaskHandle_t Task2;

extern int numPami;

/**
 * Get data from serial
 * Send data in a queue for the other thread to compute
 */
void Task1code(void *pvParameters);

/**
 * Get data from queue and compute them
 */
void Task2code(void *pvParameters);

#endif
