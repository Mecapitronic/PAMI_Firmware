#include "ESP32_Helper.h"

using namespace Printer;

namespace ESP32_Helper
{
namespace
{
uint16_t indexBuffer = 0;
uint16_t indexSeparator = 0;
const int Serial_Read_Buffer = 64;
char readBuffer[Serial_Read_Buffer];
Command cmdTmp;
//std::queue<Command> awaitingCommand;
QueueHandle_t awaitingCommand = NULL;
Preferences preferences;

void resetVar()
{
    indexBuffer = 0;
    indexSeparator = 0;
    cmdTmp.cmd = "";
    cmdTmp.size = -1;
    for (size_t i = 0; i < 8; i++)
    {
        cmdTmp.data[i] = 0;
    }
} 
// converts character array
// to string and returns it
String convertToString(char* a, int size)
{
    int i;
    String s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}
}  // namespace

void ESP32_Helper(int baud_speed, Enable printEnable, Level printLvl, Enable debugEnable)
{
    awaitingCommand = xQueueCreate(100, sizeof(Command));
    
    if (awaitingCommand == NULL)
    {
        SERIAL_DEBUG.println("Error creating the queue : awaitingCommand");
    }
    //SERIAL_DEBUG.begin(baud_speed);
    //if (SERIAL_DEBUG.available() > 0)
    //{
    //    SERIAL_DEBUG.flush();
    //}
    SERIAL_DEBUG.println();

    Printer::PrintEnable(printEnable);
    Printer::PrintLevel(printLvl);

    printHeader();
/*
    preferences.begin("Mecapi", false);
    unsigned int powerUp = preferences.getUInt("PowerUp", 0);
    powerUp++;
    println("Power Up : ", powerUp);
    preferences.putUInt("PowerUp", powerUp);
    preferences.end();
*/
    Debugger::EnableDebugger(debugEnable);
    Debugger::Initialisation();

    resetVar();
}

void printHeader()
{
    println();
    println("┌──────────────┐");
    println("├ MECAPITRONIC ┤");
    println("└──────────────┘");
    println();
    print(__DATE__);
    print(" at ");
    println(__TIME__);
    println();
}

void UpdateSerial(char tmpChar)
{
    #ifdef NO_WIFI
    while (SERIAL_DEBUG.available() > 0)
    {
        tmpChar = SERIAL_DEBUG.read();
    #endif
        if (indexBuffer < Serial_Read_Buffer)
        {
            readBuffer[indexBuffer++] = tmpChar;

            if (tmpChar == ':')
            {
                if (cmdTmp.cmd == "" && indexBuffer > 1)
                {
                    cmdTmp.cmd = String(&readBuffer[0], indexBuffer - 1);
                    indexSeparator = indexBuffer;
                }
            }
            if (tmpChar == '\n')
            {
                //SERIAL_DEBUG.print("Received : ");
                //String s = convertToString(readBuffer, indexBuffer);
                //SERIAL_DEBUG.println(s);

                int32_t index1 = indexSeparator;
                cmdTmp.size = 0;

                for (uint8_t i = indexSeparator; i < indexBuffer; i++)
                {
                    if (readBuffer[i] == ';' || readBuffer[i] == ':' || readBuffer[i] == '\n')
                    {
                        cmdTmp.data[cmdTmp.size++] = atoi(String(&readBuffer[index1], i - index1).c_str());
                        index1 = i + 1;
                    }
                }

                //print("Received", cmdTmp);

                // We first handle if the command is for the Lib
                if (cmdTmp.cmd == "DebugSteps")
                {
                    // DebugSteps:10
                    Debugger::AddSteps(cmdTmp.data[0]);
                }
                else if (cmdTmp.cmd == "DebugEnable")
                {
                    // DebugEnable:1
                    Debugger::EnableDebugger((Enable)cmdTmp.data[0]);
                }
                else if (cmdTmp.cmd == "PrintLevel")
                {
                    // PrintLevel:0
                    Printer::PrintLevel((Level)cmdTmp.data[0]);
                }
                else if (cmdTmp.cmd == "PrintEnable")
                {
                    // PrintEnable:1
                    Printer::PrintEnable((Enable)cmdTmp.data[0]);
                }
                else
                {
                    // If command is not for Lib, we sent it to the main                    
                    xQueueSend(awaitingCommand, &cmdTmp, 0);
                    //awaitingCommand.push(cmdTmp);
                }
                resetVar();
            }
        }
        else
        {
            SERIAL_DEBUG.print("Read Buffer Overflow : ");
            SERIAL_DEBUG.println(indexBuffer);
            //SERIAL_DEBUG.flush();
            resetVar();
        }
    #ifdef NO_WIFI
    }
    #endif
}

bool HasWaitingCommand() { return uxQueueMessagesWaiting(awaitingCommand) > 0; }

Command GetCommand()
{
    Command cmd;
    cmd.cmd = "";
    if (HasWaitingCommand())
    {
        if (xQueueReceive(awaitingCommand, &cmd, portTICK_PERIOD_MS * 0))
        {
            return cmd;
        }
        else
        {
            cmd.cmd = "";
        }
        //cmd = awaitingCommand.front();
        //awaitingCommand.pop();
        //print("POP", cmd);
    }
    return cmd;
}

int64_t GetTimeNowMs()
{
    micros();
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);

    int64_t time_ms = (int64_t)tv_now.tv_sec * 1000 + ((int64_t)tv_now.tv_usec / 1000);
    return time_ms;
}

int64_t GetTimeNowUs()
{
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);

    int64_t time_us = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
    return time_us;
}

int GetFromPreference(String pref, int defValue)
{
    preferences.begin("Mecapi", true);
    int value = preferences.getInt(pref.c_str(), defValue);
    print("Preferences ");
    println(pref, value);
    preferences.end();
    return value;
}

void SaveToPreference(String pref, int value)
{
    preferences.begin("Mecapi", false);
    preferences.putInt(pref.c_str(), value);
    preferences.end();
}

}  // namespace ESP32_Helper
