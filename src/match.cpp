#include "match.h"

long elapsedTime = 0;
long startTime = 0;

// Get the current state of the match
State matchState = State::MATCH_WAIT;

// Get the mode : ENABLE_TRUE is Match, ENABLE_FALSE is Train (default)
Enable matchMode = Enable::ENABLE_NONE;

void startMatch()
{
    startTime = millis();
    matchState = State::MATCH_BEGIN;
    printMatch();
}

void stopMatch()
{
    matchState = State::MATCH_STOP;
    printMatch();
}

long getMatchTime()
{
    return millis() - startTime;
}

void updateMatch()
{
    if(matchState == State::MATCH_WAIT)
    {
        // Wait start of match
    }
    else if (matchState == State::MATCH_BEGIN)
    {
        // Match started, wait for elapsed time to reach the start threshold
        elapsedTime = millis() - startTime;
        if ((elapsedTime >= TIME_START_PAMI_MATCH && matchMode == Enable::ENABLE_TRUE) || (elapsedTime >= TIME_START_PAMI_TRAIN && matchMode == Enable::ENABLE_FALSE))
        {
            matchState = State::MATCH_RUN;
            printMatch();
        }
    }
    else if ((matchState == State::MATCH_RUN) || (matchState == State::MATCH_STOP))
    {
        // PAMI still running or waiting for end of match
        elapsedTime = millis() - startTime;
        
        if ((elapsedTime >= TIME_END_PAMI_MATCH && matchMode == Enable::ENABLE_TRUE) || (elapsedTime >= TIME_END_PAMI_TRAIN && matchMode == Enable::ENABLE_FALSE))
        {
            matchState = State::MATCH_END;
            printMatch();
        }
    }
    else if(matchState == State::MATCH_END)
    {
        // End of match
    }
    else
    {
        // Not possible
    }
}

void printMatch()
{
    SERIAL_DEBUG.print("Match State : ");
    switch (matchState)
    {
        ENUM_PRINT(State::MATCH_WAIT);
        ENUM_PRINT(State::MATCH_BEGIN);
        ENUM_PRINT(State::MATCH_RUN);
        ENUM_PRINT(State::MATCH_STOP);
        ENUM_PRINT(State::MATCH_END);
    }
    
    SERIAL_DEBUG.print("Match Mode : ");
    if (matchMode == Enable::ENABLE_TRUE)
        println("Match");
    else if(matchMode == Enable::ENABLE_FALSE)
        println("Training - No starting cooldown");
    else
        println("None");
}