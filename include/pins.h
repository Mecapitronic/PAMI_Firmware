#ifndef PIN_H
#define PIN_H

#include <HardwareSerial.h>
#include <pins_arduino.h>

//******************** Pins Undefined
constexpr size_t PIN_11 = 11;

//******************** Pins UART - Serial USB Debug
// https://community.platformio.org/t/esp32-s3-native-usb-interface-and-serial-monitor-missing-first-messages/40377/10
// Serial used for USB CDC
#undef SERIAL_DEBUG
#if (ARDUINO_USB_CDC_ON_BOOT && ARDUINO_USB_MODE) || WOKWI
// HWCDC Serial;
#define SERIAL_DEBUG Serial
#else
#error "USB Serial not working"
#endif

//******************** Pins UART - Serial 0
/* Do we need this ?
#undef SOC_RX0
#define SOC_RX0 44
#undef SOC_TX0
#define SOC_TX0 43
*/

//******************** Pins Enable Power
constexpr size_t PIN_EN_MCU = 3;

//******************** Pins UART - Serial 1 Servo Dxl
#undef RX1
#undef TX1
constexpr size_t RX_SERVO = 18;
constexpr size_t TX_SERVO = 17;
#define SERIAL_SERVO Serial1

//******************** Pins SERVO
constexpr size_t PIN_SERVO_DIR = 10;

//******************** Pins Motors - Drivers
//constexpr size_t PIN_STEP_M1 = 7;
//constexpr size_t PIN_DIR_M1 = 6;

constexpr size_t PIN_STEP_M2 = 5;
constexpr size_t PIN_DIR_M2 = 4;

constexpr size_t PIN_STEP_M3 = 2;
constexpr size_t PIN_DIR_M3 = 1;

//******************** Pins TwoWire I²C - Otos
/*
#undef SDA
#define SDA 8
#undef SCL
#define SCL 9
*/

//******************** Pins Selecteur PAMI
constexpr size_t PIN_PAMI_NUM_1 = 7; // LSB
constexpr size_t PIN_PAMI_NUM_2 = 6; // MSB

#endif
