#include "match.h"

long elapsedTime = 0;
long startTime = 0;

int matchState = MATCH_WAIT;
int matchMode = MODE_MATCH;

int getMatchState()
{
    return matchState;
}

void setMatchState(int _state)
{
    matchState = _state;
}

int getMatchMode()
{
    return matchMode;
}

void setMatchMode(int _mode)
{
    matchMode = _mode;
}

void startMatch()
{
    startTime = millis();
    matchState = MATCH_BEGIN;
    println("Match Begin");
}

long getMatchTime()
{
    return millis() - startTime;
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
        if ((elapsedTime >= TIME_START_PAMI_MATCH && matchMode == MODE_MATCH) || (elapsedTime >= TIME_START_PAMI_TEST && matchMode == MODE_TEST))
        {
            setMatchState(PAMI_RUN);
            println("PAMI Run");
        }
    }
    else if ((getMatchState() == PAMI_RUN) || (getMatchState() == PAMI_STOP))
    {
        // PAMI still running or waiting for end of match
        elapsedTime = millis() - startTime;
        
        if ((elapsedTime >= TIME_END_PAMI_MATCH && matchMode == MODE_MATCH) || (elapsedTime >= TIME_END_PAMI_TEST && matchMode == MODE_TEST))
        {
            setMatchState(MATCH_END);
            println("Match End");
        }
    }
    else if(getMatchState() == MATCH_END)
    {
        // End of match
    }
    else
    {
        // Not possible
    }
}