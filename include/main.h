#ifndef MAIN_H
#define MAIN_H

#include "pin.h"
#include "motion.h"

using namespace std;

/**
 * Get data from serial
 * Send data in a queue for the other thread to compute
 */
void Task1code(void *pvParameters);

/**
 * Get data from queue and compute them
 */
void Task2code(void *pvParameters);

void waitStart();
void datumPosition(int robotNumber, int teamColor);
void match();
void strategiePAMI();

byte readRobotNumber();
bool getTeamColor();
byte getRobotNumber();

TaskHandle_t Task1;
TaskHandle_t Task2;

QueueHandle_t myQueue;
int queueSize = 500;

#endif// MAIN_H
