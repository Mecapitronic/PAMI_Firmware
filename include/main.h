#ifndef MAIN_H
#define MAIN_H

#include "ESP32_Hardware.h"
#include "ESP32_Helper.h"

#include "pins.h"
#include "sensors.h"
#include "motion.h"

#include <Wire.h>
#include <math.h>

using namespace std;
using namespace Printer;

int numPami = -1;

TaskHandle_t Task1;
TaskHandle_t Task2;

extern int numPami;

/**
 * Get data from serial
 * Send data in a queue for the other thread to compute
 */
void TaskMatch(void *pvParameters);

/**
 * Get data from queue and compute them
 */
void TaskSerial(void *pvParameters);

void Blink();

#endif
