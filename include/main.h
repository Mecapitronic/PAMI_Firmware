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

extern int numPami;

void TaskLoop(void *pvParameters);

void TaskMatch(void *pvParameters);
void TaskTeleplot(void *pvParameters);
void TaskHandleCommand(void *pvParameters);

#endif
