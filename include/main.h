#ifndef MAIN_H
#define MAIN_H

#include "ESP32_Helper.h"

#include "pin.h"
#include "match.h"
#include "ihm.h"
#include "sensors.h"
#include "motion.h"
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
