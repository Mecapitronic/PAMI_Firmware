#ifndef MATCH_H
#define MATCH_H

#include "ESP32_Helper.h"
using namespace Printer;
using namespace std;

#define TIME_START_PAMI_MATCH 90000
#define TIME_START_PAMI_TEST 1000
#define TIME_END_PAMI_TEST TIME_START_PAMI_TEST + 10000
#define TIME_END_PAMI_MATCH TIME_START_PAMI_MATCH + 10000

#define MATCH_WAIT 0
#define MATCH_BEGIN 1
#define PAMI_RUN 2
#define PAMI_STOP 3
#define MATCH_END 4

#define MODE_MATCH 0
#define MODE_TEST 1


int getMatchState();
void setMatchState(int _state);

int getMatchMode();
void setMatchMode(int _mode);

void startMatch();
long getMatchTime();
void updateMatch();

#endif
