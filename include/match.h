#ifndef MATCH_H
#define MATCH_H

#include "ESP32_Helper.h"
using namespace Printer;
using namespace std;

#define TIME_START_PAMI 5000 // 90000 //TODO
#define TIME_END_PAMI TIME_START_PAMI + 100000 // 10000 //TODO

#define MATCH_WAIT 0
#define MATCH_BEGIN 1
#define PAMI_RUN 2
#define PAMI_STOP 3

int getMatchState();
void setMatchState(int _state);
void startMatch();
void updateMatch();

#endif// MATCH_H
