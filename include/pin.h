#ifndef PIN_H
#define PIN_H

#include <Arduino.h>

// Pins Motors - Drivers

#define MS1         18
#define MS2         19

#define ENABLE_G    32
#define STEP_G      33
#define DIR_G       25

#define ENABLE_D    26
#define STEP_D      27
#define DIR_D       14

#define SHARP_1 34 // GPIO34 = ADC1_6
//#define DETECT_2 23

// Led Bi-Couleur
#define LED_1_A 22
#define LED_1_B 23

// Tirette
#define PIN_TIRETTE 12
// Color
#define PIN_COLOR 21

//#undef Serial
//#define Serial Serial2

#endif
