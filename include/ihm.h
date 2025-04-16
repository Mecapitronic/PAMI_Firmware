#ifndef IHM_H
#define IHM_H

#include "ESP32_Helper.h"
#include "pin.h"
#include "match.h"
#include <FastLED.h>

using namespace Printer;
using namespace std;

// TEAM_BLUE = 0
// TEAM_YELLOW = 1
enum class Team 
{
    TEAM_BLUE,
    TEAM_YELLOW,
    TEAM_NONE
};

extern Team team;
extern Enable tirette;
extern int switchMode;
extern int bauReady;

extern CRGB led[1];

extern bool useBlink;

void InitIHM();
void UpdateHMI();
void UpdateBAU();

void Blink();

void PrintTeam();
void PrintSwitch();
void PrintBAU();
void PrintStart();

#endif