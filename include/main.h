#ifndef MAIN_H
#define MAIN_H

#include <Arduino.h>

using namespace std;

// defines pins
#define stepPin 12
#define dirPin 11

/**
 * Get data from serial
 * Send data in a queue for the other thread to compute
 */
void Task1code(void *pvParameters);

/**
 * Get data from queue and compute them
 */
void Task2code(void *pvParameters);

TaskHandle_t Task1;
TaskHandle_t Task2;

QueueHandle_t myQueue;
int queueSize = 500;

#endif
