#include "main.h"

Preferences preferences;

void setup()
{
  Serial.end();
  Serial.begin(921600);
  delay(1000);
  Serial.println();
  Serial.println("PAMI Firmware");

  // Sets the Sensor pins as Inputs
  // pinMode(DETECT_1, INPUT_PULLUP);
  // pinMode(DETECT_2, INPUT_PULLUP);

  // Tirette
  pinMode(PIN_TIRETTE, INPUT_PULLUP);

  // Switch Color
  pinMode(PIN_COLOR, INPUT_PULLUP);

  initMotion();

  // Led Bi-Color
  pinMode(LED_1_A, OUTPUT);
  digitalWrite(LED_1_A, LOW);
  pinMode(LED_1_B, OUTPUT);
  digitalWrite(LED_1_B, LOW);

  ESP32_Helper::ESP32_Helper();

  int speedPref = ESP32_Helper::GetFromPreference("Speed",0);
  int accelPref = ESP32_Helper::GetFromPreference("Accel",0);
    
  if(speedPref != 0)
  {
    setMaxSpeed(speedPref);
    println("Speed : ", speedPref);
  }
  if(accelPref != 0)
  {  setAcceleration(accelPref);
    println("Accel : ", accelPref);
  }

  /* Task function. */
  /* name of task. */
  /* Stack size of task */
  /* parameter of the task */
  /* priority of the task */
  /* Task handle to keep track of created task */
  /* pin task to core 0 */
  xTaskCreatePinnedToCore(Task1code, "Task1", 20000, NULL, 10, &Task1, 0);
  xTaskCreatePinnedToCore(Task2code, "Task2", 20000, NULL, 5, &Task2, 1);
}

bool useBlink = true;
int ledState = LOW;
unsigned long previousMillisLED = 0;
unsigned long intervalLED = 1000;
unsigned long currentMillisLED = 0;

void loop()
{
  updateMatch();
  if (getMatchState() != PAMI_STOP && getMatchState() != MATCH_END && (motor_D.isRunning() || motor_G.isRunning()))
  {
    enableMotors();
    updateMotors();
  }
  else
  {
    disableMotors();
  }
  Blink();
}

int lastMatchTime = 0;
// Note the 1 Tick delay, this is need  so the watchdog doesn't get confused
void Task1code(void *pvParameters)
{
  Serial.println("Start Task1code");

  while (1)
  {
    try
    {
      // Attente du démarrage du match par la tirette
      if (getMatchState() == MATCH_WAIT)
      {
        TeamColor teamColorTmp = (TeamColor)digitalRead(PIN_COLOR);

        if (teamColorTmp != teamColor)
        {
          PrintTeamColor(teamColorTmp);
          teamColor = teamColorTmp;

          // Save Y position and orientation
          setCurrentY(CENTER_POSITION_MM);
          setCurrentRot(90);
          if (teamColor == TEAM_YELLOW)
          {
            if (numPami == 1)
              setCurrentX(1102);
            else if (numPami == 2)
              setCurrentX(1216);
            else if (numPami == 3)
              setCurrentX(1330);
            else if (numPami == 4)
              setCurrentX(1444);
            else
              println("ERROR robot number");
          }
          else if (teamColor == TEAM_BLUE)
          {
            if (numPami == 1)
              setCurrentX(1898);
            else if (numPami == 2)
              setCurrentX(1784);
            else if (numPami == 3)
              setCurrentX(1670);
            else if (numPami == 4)
              setCurrentX(1556);
            else
              println("ERROR robot number");
          }
        }

        Enable tiretteTmp = (Enable)!digitalRead(PIN_TIRETTE);
        if (tiretteTmp != tirette)
        {
          if (tirette == ENABLE_NONE)
          {
            if (tiretteTmp == ENABLE_TRUE)
            {
              println("Tirette Présente au démarrage");
              setMatchMode(MODE_MATCH);
              intervalLED = 500;
            }
            else if (tiretteTmp == ENABLE_FALSE)
            {
              println("Tirette Absente au démarrage");
              setMatchMode(MODE_TEST);
              intervalLED = 200;
            }
          }
          else
          {
            if (tiretteTmp == ENABLE_TRUE)
            {
              println("Tirette Insérée");
              intervalLED = 500;
            }
            else if (tiretteTmp == ENABLE_FALSE)
            {
              println("Tirette Enlevée");
              intervalLED = 1000;
              startMatch();
            }
          }
          tirette = tiretteTmp;
        }
      }

      // Match en cours
      if (getMatchState() == MATCH_BEGIN)
      {
        if(lastMatchTime != (int)(getMatchTime()/1000))
        {
          println("Match Time : ", (int)(getMatchTime()/1000));
          lastMatchTime=(int)(getMatchTime()/1000);
        }
      }

      // Démarrage des PAMI // TODO : change values of points
      if (getMatchState() == PAMI_RUN)
      {
        setOpponentChecking(true);
        if (numPami == 1)
        {
          if (teamColor == TEAM_YELLOW)
          {
            //goTo(1102, 650);
            goTo(1102, 150);
            goTo(763, 150);
            setOpponentChecking(false);
            goTo(763, 20);
          }
          else
          {
            //goTo(3000 - 1102, 650); // 1 roue tourne plus vite que l'autre = 1 arc de cercle
            goTo(3000 - 1102, 150);
            goTo(3000 - 763, 150);
            setOpponentChecking(false);
            goTo(3000 - 763, 20);
          }
        }
        else if (numPami == 2)
        {
          if (teamColor == TEAM_YELLOW)
          {
            goTo(1216, 475);
            float accel = getAcceleration();
            setAcceleration(2000);
            float speed = getMaxSpeed();
            setMaxSpeed(2000);
            turnTo(450, 475);
            setAcceleration(accel);
            setMaxSpeed(speed);
            goTo(450, 475);
            setOpponentChecking(false);
            goTo(20, 475);
          }
          else
          {
            goTo(3000 - 1216, 475);
            float accel = getAcceleration();
            setAcceleration(2000);
            float speed = getMaxSpeed();
            setMaxSpeed(2000);
            turnTo(3000 - 450, 475);
            setAcceleration(accel);
            setMaxSpeed(speed);
            goTo(3000 - 450, 475);
            setOpponentChecking(false);
            goTo(3000 - 20, 475);
          }
        }
        else if (numPami == 3)
        {
          if (teamColor == TEAM_YELLOW)
          {
            goTo(1330, 1400);
            goTo(400, 1600);
            setOpponentChecking(false);
            //goTo(400, 1600);
            goTo(350, 1650);
          }
          else
          {
            goTo(3000 - 1330, 1400);
            goTo(3000 - 400, 1600);
            setOpponentChecking(false);
            //goTo(3000 - 400, 1600);
            goTo(3000 - 350, 1650);
          }
        }
        else if (numPami == 4)
        {
          if (teamColor == TEAM_YELLOW)
          {
            goTo(1444, 1000);
            goTo(2500, 1000);
            setOpponentChecking(false);
            goTo(2800, 1000);
          }
          else
          {
            goTo(3000 - 1444, 1000);
            goTo(3000 - 2500, 1000);
            setOpponentChecking(false);
            goTo(3000 - 2800, 1000);
          }
        }
        println("PAMI Stop");
        setMatchState(PAMI_STOP);
      }

      // Arrêt des PAMI
      if (getMatchState() == PAMI_STOP)
      {
        // Wait for end of match
      }
      
      // Fin du match
     if (getMatchState() == MATCH_END)
     {
        useBlink = false;
     }

      // Check if we get commands from operator via debug serial
      if (ESP32_Helper::HasWaitingCommand())
      {
        Command cmd = ESP32_Helper::GetCommand();

        if (cmd.cmd.startsWith("PamiNumber"))
        {
          // print("Pami : ", cmd);
          if (cmd.size > 0)
          {            
            print("Changing Num PAMI from : ", numPami);
            println(" to : ",cmd.data[0]);
            ESP32_Helper::SaveToPreference("PamiNumber",cmd.data[0]);
            vTaskDelay(500);
            ESP.restart();
          }
        }
        if (cmd.cmd.startsWith("Speed"))
        {
          // print("Speed : ", cmd);
          if (cmd.size > 0)
          {
            setMaxSpeed(cmd.data[0]);
            ESP32_Helper::SaveToPreference("Speed",cmd.data[0]);
            println("Speed : ", getMaxSpeed());
          }
          println("Motor D speed:", motor_D.speed());
          println("Motor G speed:", motor_G.speed());
        }
        if (cmd.cmd.startsWith("Accel"))
        {
          // print("Accel : ", cmd);
          if (cmd.size > 0)
          {
            setAcceleration(cmd.data[0]);
            ESP32_Helper::SaveToPreference("Accel",cmd.data[0]);
            println("Accel : ", getAcceleration());
          }
          println("Motor D accel:", motor_D.acceleration());
          println("Motor G accel:", motor_G.acceleration());
        }
        if (cmd.cmd.startsWith("Pulse"))
        {
          // print("Pulse : ", cmd);
          if (cmd.size > 0)
          {
            motor_D.setMinPulseWidth(cmd.data[0]);
            motor_G.setMinPulseWidth(cmd.data[0]);
          }
          println("setMinPulseWidth:", cmd.data[0]);
        }
        if (cmd.cmd.startsWith("Go"))
        {
          // print("Go : ", cmd);
          if (cmd.size == 1)
            go(cmd.data[0]);
          if (cmd.size == 2)
            goTo(cmd.data[0], cmd.data[1]);
          if (cmd.size == 3)
            goTo(cmd.data[0], cmd.data[1], cmd.data[2]);
        }
        if (cmd.cmd.startsWith("Turn"))
        {
          // print("Turn : ", cmd);
          if (cmd.size > 0)
            turn(cmd.data[0]);
        }
        if (cmd.cmd.startsWith("Motor"))
        {
          println("Motor D:");
          println("speed:", motor_D.speed());
          println("acceleration:", motor_D.acceleration());
          println("distanceToGo:", (int)motor_D.distanceToGo());
          println("targetPosition:", (int)motor_D.targetPosition());
          println("currentPosition:", (int)motor_D.currentPosition());
          // println("computeNewSpeed:",(long)motor_D.computeNewSpeed());
          println("-----");
          println("Motor G:");
          println("speed:", motor_G.speed());
          println("acceleration:", motor_G.acceleration());
          println("distanceToGo:", (int)motor_G.distanceToGo());
          println("targetPosition:", (int)motor_G.targetPosition());
          println("currentPosition:", (int)motor_G.currentPosition());
          // println("computeNewSpeed:",(long)motor_G.computeNewSpeed());
          println("-----");
        }
        if (cmd.cmd.startsWith("Blink"))
        {
          // print("Blink : ", cmd);
          if (cmd.size > 0)
            useBlink = cmd.data[0];
          println("Blink : ", useBlink);
        }
        if (cmd.cmd.startsWith("stepMultiplier"))
        {
          // print("stepMultiplier : ", cmd);
          if (cmd.size > 0)
            setStepMode(StepMode(cmd.data[0]));
          println("stepMultiplier : ", StepMode(cmd.data[0]));
        }
      }
    }
    catch (std::exception const &e)
    {
      Serial.print("error : ");
      Serial.println(e.what());
    }
    vTaskDelay(1);
  }
}

unsigned long previousMillisWifi = 0;
unsigned long previousMillisServer = 0;
unsigned long intervalWifi = 5000;
unsigned long intervalServer = 5000;
unsigned long currentMillisWifi = 0;
unsigned long currentMillisServer = 0;

int64_t lastSendSerialTime = millis();

Pose lastPosition = {0, 0, 0};

// Note the 1 Tick delay, this is need so the watchdog doesn't get confused
void Task2code(void *pvParameters)
{
  Serial.println("Start Task2code");

  while (1)
  {
    try
    {
      ESP32_Helper::UpdateSerial();

            if (millis() - lastSendSerialTime > 500)
            {
                lastSendSerialTime = millis();
                Pose p = getCurrentPose();

                if ((int)lastPosition.x != (int)getCurrentPose().x ||
                    (int)lastPosition.y != (int)getCurrentPose().y ||
                    (int)(lastPosition.rot) != (int)(getCurrentPose().rot))
                {
                  PolarPoint p = {0,0,0,0,0};
                  p.x = getCurrentPose().x;
                  p.y = getCurrentPose().y;
                    teleplot("pos", p, (int)(getCurrentPose().rot), LEVEL_WARN);
                    lastPosition = getCurrentPose();
                }
                // teleplot("mapBoundaries", MapBoundaries, 4, LEVEL_WARN);
                // teleplot("robot", robot.GetPosition(), LEVEL_WARN);
            }

      currentMillisWifi = millis();
      // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
      if ((WiFi.status() != WL_CONNECTED) && (currentMillisWifi - previousMillisWifi >= intervalWifi))
      {
        Serial.println("Reconnecting to WiFi...");
        //WiFi.disconnect();
        WiFi.reconnect();
        previousMillisWifi = currentMillisWifi;
      }

      currentMillisServer = millis();
      if (WiFi.status() == WL_CONNECTED)
      {
        if (!client.connected() && (currentMillisServer - previousMillisServer >= intervalServer))
        {
          client.stop();
          if (client.connect("192.168.137.1", 20240))
          {
            Serial.println("Connected to server !");
            println("PAMI : ", numPami, " connected !");
          }
          else
          {
            Serial.println("Connection to server failed");
          }
          previousMillisServer = currentMillisServer;
        }
      }
    }
    catch (std::exception const &e)
    {
      Serial.print("error : ");
      Serial.println(e.what());
    }
    vTaskDelay(1);
    // vTaskDelay(10 / portTICK_PERIOD_MS);
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
      if (ledState == LOW)
        ledState = HIGH;
      else
        ledState = LOW;
      if (teamColor == TEAM_BLUE)
      {
        digitalWrite(LED_1_A, LOW);
        digitalWrite(LED_1_B, ledState);
      }
      else if (teamColor == TEAM_YELLOW)
      {
        digitalWrite(LED_1_B, LOW);
        digitalWrite(LED_1_A, ledState);
      }
    }
  }
  else if (ledState == LOW)
  {
    ledState = HIGH;
    if (teamColor == TEAM_BLUE)
    {
      digitalWrite(LED_1_A, LOW);
      digitalWrite(LED_1_B, ledState);
    }
    else if (teamColor == TEAM_YELLOW)
    {
      digitalWrite(LED_1_B, LOW);
      digitalWrite(LED_1_A, ledState);
    }
  }
}