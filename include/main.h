#ifndef MAIN_H
#define MAIN_H

#include "ESP32_Helper.h"

using namespace Printer;
using namespace std;

#include "pin.h"
#include "motion.h"

#include <WiFi.h>

#ifdef WITH_WIFI
// Load Wi-Fi library
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WebSerial.h>

#include "BasicOTA.hpp"

BasicOTA OTA;

// Set web server port number to 80
AsyncWebServer server(80);
#endif

const int max_pami = 4;

const String wifi_ssid = "Mecapitronic";
const String wifi_password = "Geoffroy";
const String wifi_mac_pami[max_pami] = {"94:3C:C6:38:B2:F4","00:00:00:00:00:00","00:00:00:00:00:00","00:00:00:00:00:00"};

const String ap_ssid = "MECAPI_PAMI";
const String ap_mac_pami[max_pami] = {"94:3C:C6:38:B2:F5","00:00:00:00:00:00","00:00:00:00:00:00","00:00:00:00:00:00"};
const String ap_password = "Geoffroy"; // Only works when the length of passphrase is >= 8 characters


TeamColor teamColor = TEAM_NONE;
Enable tirette = ENABLE_NONE;
int numPami = 0;

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

// Callback
void recvMsg(uint8_t *data, size_t len);
void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info);
void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info);

#endif// MAIN_H
