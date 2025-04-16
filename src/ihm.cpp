#include "ihm.h"

Team team = Team::TEAM_NONE;
Enable tirette = Enable::ENABLE_NONE;
int switchMode = -1;
int bauReady = -1;
int bauReadyPrev = -1;

CRGB led[1];

bool useBlink = true;
int ledState = LOW;
unsigned long previousMillisLED = 0;
unsigned long intervalLED = 1000;
unsigned long currentMillisLED = 0;

void InitIHM()
{    
  // Tirette
  pinMode(PIN_START, INPUT_PULLUP);

  // Switch Color
  pinMode(PIN_TEAM, INPUT_PULLUP);

  // Switch Switch
  pinMode(PIN_SWITCH, INPUT_PULLUP);

  // Boutton Arret d'Urgence
  pinMode(PIN_BAU, INPUT);

  tirette = (Enable)!digitalRead(PIN_START);
  if (tirette == Enable::ENABLE_TRUE)
  {
      println("Tirette : Présente au démarrage");
      matchMode = Enable::ENABLE_TRUE;
      intervalLED = 500;
  }
  else if (tirette == Enable::ENABLE_FALSE)
  {
      println("Tirette : Absente au démarrage");
      matchMode = Enable::ENABLE_FALSE;
      intervalLED = 200;
  }  
  printMatch();
  UpdateHMI();
  UpdateBAU();

  FastLED.addLeds<WS2812, PIN_RGB_LED, RGB>(led, 1);
  led[0] = CRGB::Black;
  FastLED.setBrightness(50);
  FastLED.show();
}

void UpdateHMI()
{
    // Lecture du bouton Team Yellow / Blue
    Team teamTmp = (Team)digitalRead(PIN_TEAM);
    if (teamTmp != team)
    {
        team = teamTmp;
        PrintTeam();
    }

    // Lecture du bouton Switch TEST / OK
    int switchTmp = digitalRead(PIN_SWITCH);
    if (switchTmp != switchMode)
    {
        switchMode = switchTmp;
        PrintSwitch();
    }

    // Lecture de la tirette de démarrage
    Enable tiretteTmp = (Enable)!digitalRead(PIN_START);
    if (tiretteTmp != tirette)
    {
        if (tiretteTmp == Enable::ENABLE_TRUE)
        {
            intervalLED = 500;
        }
        else if (tiretteTmp == Enable::ENABLE_FALSE)
        {
            intervalLED = 1000;
            startMatch();
        }
        tirette = tiretteTmp;
        PrintStart();
    }
    
    if(bauReadyPrev != bauReady)
    {
        bauReadyPrev = bauReady;
        PrintBAU();
    }
}

void UpdateBAU()
{
    // Lecture du BAU
    int bauTmp = digitalRead(PIN_BAU);
    if (bauTmp != bauReady)
    {
        bauReady = bauTmp;
    }
}

void Blink()
{
  if (useBlink)
  {
    currentMillisLED = millis();
    if (currentMillisLED - previousMillisLED >= intervalLED)
    {
      previousMillisLED = currentMillisLED;
      ledState = !ledState;
      if(ledState)
      {
        if (team == Team::TEAM_YELLOW)
        {
          led[0] = CRGB::Gold;
        }
        else
        {
          led[0] = CRGB::Blue;
        }
      }
      else
      {
        if (bauReady)
        {
          led[0] = CRGB::Black;
        }
        else
        {
          led[0] = CRGB::Red;
        }
      }    
      FastLED.show();
    }
  }
  else
  {
    ledState = HIGH;
    if(!bauReady)
    {
      if(led[0] != CRGB::Red)
      {
        led[0] = CRGB::Red;
        FastLED.show();
      }
    }
    else
    {
      if (team == Team::TEAM_YELLOW)
      {
        if(led[0] != CRGB::Gold)
        {
          led[0] = CRGB::Gold;
          FastLED.show();
        }
      }
      else
      {
        if(led[0] != CRGB::Blue)
        {
          led[0] = CRGB::Blue;
          FastLED.show();
        }
      }
    }
  }
}

void PrintTeam()
{
    print("Team    : ");
    if(team == Team::TEAM_BLUE)
        println("BLUE");
    else if (team == Team::TEAM_YELLOW)
        println("YELLOW");
}
    
void PrintSwitch()
{
    print("Switch  : ");
    if(switchMode)
        println("OK");
    else
        println("TEST");
}

void PrintBAU()
{
    print("BAU     : ");
    if(bauReady)
        println("Retiré");
    else
        println("Enclenché");
}

void PrintStart()
{
    print("Tirette : ");
    if (tirette == Enable::ENABLE_TRUE)
        println("Insérée");
    else if (tirette == Enable::ENABLE_FALSE)
        println("Enlevée");
}