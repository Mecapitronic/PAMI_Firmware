#include "match.h"

long elapsedTime = 0;
long startTime = 0;

int matchState = MATCH_WAIT;

int getMatchState()
{
    return matchState;
}

void setMatchState(int _state)
{
    matchState = _state;
}

void startMatch()
{
    startTime = millis();
    matchState = MATCH_BEGIN;
    println("Match Begin");
}

void updateMatch()
{
    if(getMatchState() == MATCH_WAIT)
    {
        // Wait start of match
    }
    else if (getMatchState() == MATCH_BEGIN)
    {
        // Match running
        elapsedTime = millis() - startTime;
        if (elapsedTime >= TIME_START_PAMI)
        {
            setMatchState(PAMI_RUN);
            println("PAMI Run");
        }
    }
    else if (getMatchState() == PAMI_RUN)
    {
        // PAMI running
        elapsedTime = millis() - startTime;
    
        if (elapsedTime >= TIME_END_PAMI)
        {
            setMatchState(PAMI_STOP);
            println("Match End");
        }
    }
    else if(getMatchState() == PAMI_STOP)
    {
        // End of match
    }
    else
    {
        // Not possible
    }
}