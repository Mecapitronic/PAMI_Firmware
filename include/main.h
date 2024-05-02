#ifndef MAIN_H
#define MAIN_H

using namespace std;

#include "pin.h"
#include "motion.h"

TaskHandle_t Task1;
TaskHandle_t Task2;

/**
 * Get data from serial
 * Send data in a queue for the other thread to compute
 */
void Task1code(void *pvParameters);

/**
 * Get data from queue and compute them
 */
void Task2code(void *pvParameters);


#endif// MAIN_H
