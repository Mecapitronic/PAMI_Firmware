#ifndef MATCH_H
#define MATCH_H

#include "ESP32_Helper.h"

#define TIME_START_PAMI_MATCH 85000
#define TIME_START_PAMI_TRAIN 1000
#define TIME_END_PAMI_TRAIN TIME_START_PAMI_TRAIN + 15000
#define TIME_END_PAMI_MATCH TIME_START_PAMI_MATCH + 15000

// WAIT BEGIN RUN STOP END
enum class State
{
    MATCH_WAIT,
    MATCH_BEGIN,
    MATCH_RUN,
    MATCH_STOP,
    MATCH_END
};

extern State matchState;
extern Enable matchMode;

void startMatch();
void stopMatch();
void resetMatch();
long getMatchTimeSec();
long getMatchTimeMs();
void updateMatch();
void printMatch();

#endif
