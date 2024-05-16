/**
 * @file ESP32_Helper.h
 * @author Mecapitronic (mecapitronic@gmail.com)
 * @brief
 * @date 2023-07-25
 */
#ifndef ESP32_HELPER_H
#define ESP32_HELPER_H

extern int numPami;

#ifdef WITH_WIFI
#include <WiFi.h>
extern WiFiClient client;
#define SERIAL_DEBUG client
#endif

#ifdef NO_WIFI
#define SERIAL_DEBUG Serial
#endif

#ifndef SERIAL_DEBUG
    #define SERIAL_DEBUG Serial
#endif

#include "Structure.h"
#include "Printer.h"
#include "Debugger.h"
#include "Preferences.h"

namespace ESP32_Helper
{

const int max_pami = 4;

const String wifi_ssid = "Mecapitronic";
const String wifi_password = "Geoffroy";
const String wifi_mac_pami[max_pami] = {"94:3C:C6:38:B2:F4","94:3C:C6:37:83:5C","94:3C:C6:38:BE:E8","94:3C:C6:38:3E:A0"};

const String ap_ssid = "MECAPI_PAMI";
const String ap_mac_pami[max_pami] = {"94:3C:C6:38:B2:F5","94:3C:C6:37:83:5D","94:3C:C6:38:BE:E9","94:3C:C6:38:3E:A1"};
const String ap_password = "Geoffroy"; // Only works when the length of passphrase is >= 8 characters


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
void UpdateSerial();
bool HasWaitingCommand();
Command GetCommand();
int GetNumPami();

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

}
#endif
