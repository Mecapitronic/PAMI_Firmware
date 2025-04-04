#ifndef PIN_H
#define PIN_H

#include <cstddef>
#include <pins_arduino.h>
//#include <HardwareSerial.h>

//******************** Pins Undefined
constexpr size_t PIN_10 = 10;
constexpr size_t PIN_11 = 11;

//******************** Pins UART - Serial
/* Do we need this ?
#undef SOC_RX0
#define SOC_RX0 44
#undef SOC_TX0
#define SOC_TX0 43

#undef RX1
#define RX1 18
#undef TX1
#define TX1 17
*/
//******************** Pins Motors - Drivers
constexpr size_t PIN_STEP_M1 = 7;
constexpr size_t PIN_DIR_M1 = 6;

constexpr size_t PIN_STEP_M2 = 5;
constexpr size_t PIN_DIR_M2 = 4;

constexpr size_t PIN_STEP_M3 = 2;
constexpr size_t PIN_DIR_M3 = 1;

constexpr size_t PIN_EN_MCU = 3;

//******************** Pins TwoWire I²C - Otos

#undef SDA
#define SDA 8
#undef SCL
#define SCL 9

//******************** Pins LED - RGB
constexpr size_t PIN_RGB_LED = 38;
constexpr size_t PIN_WS2812_LED = 12;

//******************** Pins IHM
constexpr size_t PIN_MODE = 14;
constexpr size_t PIN_TEAM = 13;
constexpr size_t PIN_BAU = 15;
constexpr size_t PIN_START = 16;

//******************** Pins Selecteur PAMI
constexpr size_t PIN_PAMI_NUM_1 = 7;
constexpr size_t PIN_PAMI_NUM_2 = 6;

#endif
