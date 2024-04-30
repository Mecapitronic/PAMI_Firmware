/**
 * @file ESP32_Helper.h
 * @author Mecapitronic (mecapitronic@gmail.com)
 * @brief
 * @date 2023-07-25
 */
#ifndef ESP32_HELPER_H
#define ESP32_HELPER_H

#include "Structure.h"
#include "Printer.h"
#include "Debugger.h"
#include "Preferences.h"

#ifdef WITH_WIFI
#include <WebSerial.h>
#define SERIAL_DEBUG WebSerial
#endif

#ifdef NO_WIFI
#define SERIAL_DEBUG Serial
#endif

namespace ESP32_Helper
{
// ESP32 Serial Bauds rates
// static const unsigned long default_rates[] = {300, 600, 1200, 2400, 4800, 9600, 19200, 38400, 57600, 74880,
// 115200, 230400, 256000, 460800, 921600, 1843200, 3686400};

/**
 * Initialize serial for PC communication
 */
void ESP32_Helper(int baud_speed = 921600, Enable printEnable = ENABLE_TRUE,
                  Level printLvl = LEVEL_VERBOSE, Enable debugEnable = ENABLE_FALSE);

void printHeader();

/**
 * Check for commands send on debugging serial plugged to a computer
 */
void UpdateSerial(char tmpChar = '\0');
bool HasWaitingCommand();
Command GetCommand();

/**
 * @brief Get the current time with miliseconds precision
 *
 * @return int64_t current time in miliseconds
 */
int64_t GetTimeNowMs();

/**
 * @brief Get the current time with microsecond precision
 *
 * @return int64_t current time in microseconds
 */
int64_t GetTimeNowUs();


int GetFromPreference(String pref, int defValue=0);
void SaveToPreference(String pref, int value);

}  // namespace ESP32_Helper
#endif// ESP32_HELPER_H
