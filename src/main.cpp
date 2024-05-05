#include "main.h"

Preferences preferences;

void setup()
{
  Serial.end();
  Serial.begin(921600);
  delay(1000);
  Serial.println("PAMI Firmware");
  
  // Sets the Sensor pins as Inputs
  pinMode(DETECT_1, INPUT_PULLUP);
  pinMode(DETECT_2, INPUT_PULLUP);

  pinMode(PIN_TIRETTE, INPUT_PULLUP);
  pinMode(PIN_COLOR, INPUT_PULLUP);

  initMotion();

  ESP32_Helper::ESP32_Helper();

  delay(1000);

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

void loop()
{
  // Do not put code in the loop() when using freeRTOS.
  // loop() is the only task that is guaranteed to not be ran per tasking iteration.
  //delay(1000);

      updateMatch();
      if (getMatchState() != PAMI_STOP && (motor_D.isRunning() || motor_G.isRunning()))
      {
        enableMotors();
        updateMotors();
      }
      else
      {
        disableMotors();
      }
}

// Note the 1 Tick delay, this is need  so the watchdog doesn't get confused
void Task1code(void *pvParameters)
{
  Serial.println("Start Task1code");

  while (1)
  {
    try
    {
      // Attente du démarrage du match par la tirette
      if(getMatchState() == MATCH_WAIT)
      {
        TeamColor teamColorTmp = (TeamColor)digitalRead(PIN_COLOR);
        if (teamColorTmp != teamColor)
        {
          if(teamColorTmp == TEAM_BLUE)
            println("Color Team BLUE");
          else if (teamColorTmp == TEAM_YELLOW)
            println("Color Team YELLOW");
          teamColor = teamColorTmp;
        }
        
        Enable tiretteTmp = (Enable)digitalRead(PIN_TIRETTE);
        if (tiretteTmp != tirette)
        {
          if(tirette == ENABLE_NONE)
          {
            if(tiretteTmp == ENABLE_TRUE)
              println("Tirette Présente au démarrage");
            else if (tiretteTmp == ENABLE_FALSE)
              println("Tirette Absente au démarrage");
          }
          else
          {
            if(tiretteTmp == ENABLE_TRUE)
            {
                  println("Tirette Insérée");
                  // Datum at low Speed
                  setMaxSpeed(DATUM_SPEED);
                  setAcceleration(DATUM_ACCELERATION);
                  // Datum Y
                  go(-100);
                  // Save Y position and orientation
                  setCurrentY(CENTER_POSITION_MM);
                  setCurrentRot(270);

                  if (teamColor == TEAM_BLUE)
                  {
                    // Orientate robot
                    goTo(0, 80, 0);
                    go(-100);
                    // SaveX position and orientation
                    setCurrentX(1050 + CENTER_POSITION_MM);
                    setCurrentRot(0);
/*
                    if (numPami == 1)
                      goTo(1120, 80, 270); // Go to safe position
                    else if (numPami == 2)
                      goTo(1120 + 130, 80, 270); // Go to safe position
                    else if (numPami == 3)
                      goTo(1120 + 260, 80, 270); // Go to safe position
                    else
                      println("ERROR robot number");*/
                  }
                  else if (teamColor == TEAM_YELLOW)
                  {
                    goTo(0, 80, 180);
                    go(-100);
                    // SaveX position and orientation
                    setCurrentX(1950 - CENTER_POSITION_MM);
                    setCurrentRot(180);
/*
                    if (numPami == 1)
                      goTo(1880, 80, 270); // Go to safe position
                    else if (numPami == 2)
                      goTo(1880 - 130, 80, 270); // Go to safe position
                    else if (numPami == 3)
                      goTo(1880 - 260, 80, 270); // Go to safe position
                    else
                      println("ERROR robot number");
                      */
                  }
                  setMaxSpeed(MAX_SPEED);
                  setAcceleration(MAX_ACCELERATION);                
            }
            else if (tiretteTmp == ENABLE_FALSE)
            {
                println("Tirette Enlevée");
                startMatch();
            }
          }
          tirette = tiretteTmp;
        }
      }
      
      // Match en cours
      if(getMatchState() == MATCH_BEGIN)
      {
      }

      // Démarrage des PAMI // TODO : change values of points
      if(getMatchState() == PAMI_RUN)
      {        
          setOpponentChecking(true);
          //if (numPami == 1)
          {
            if (teamColor == TEAM_BLUE)
            {
              goTo(750, 180);
              setOpponentChecking(false);
              goTo(750, 0);
            }
            else
            {
              goTo(3000 - 750, 180);
              setOpponentChecking(false);
              goTo(3000 - 750, 0);
            }
          }
          //else if (numPami == 2)
          {
            if (teamColor == TEAM_BLUE)
            {
              goTo(1200, 300);
              goTo(600, 300);
              setOpponentChecking(false);
              goTo(400, 300);
            }
            else
            {
              goTo(3000 - 1200, 300);
              goTo(3000 - 600, 300);
              setOpponentChecking(false);
              goTo(3000 - 400, 300);
            }
          }
          //else if (numPami == 3)
          {
            if (teamColor == TEAM_BLUE)
            {
              goTo(1350, 450);
              goTo(400, 550);
              setOpponentChecking(false);
              goTo(0, 550);
            }
            else
            {
              goTo(3000 - 1350, 450);
              goTo(3000 - 400, 550);
              setOpponentChecking(false);
              goTo(3000 - 0, 550);
            }
          }
      }

      
      // Arrêt des PAMI et fin du match
      if(getMatchState() == PAMI_STOP)
      {
      }

      // Check if we get commands from operator via debug serial
      if (ESP32_Helper::HasWaitingCommand())
      {
        Command cmd = ESP32_Helper::GetCommand();

        if (cmd.cmd.startsWith("Speed"))
        {
          // print("Speed : ", cmd);
          if (cmd.size > 0)
            setMaxSpeed(cmd.data[0]);
          println("Motor D speed:", motor_D.speed());
          println("Motor G speed:", motor_G.speed());
        }
        if (cmd.cmd.startsWith("Accel"))
        {
          // print("Accel : ", cmd);
          if (cmd.size > 0)
            setAcceleration(cmd.data[0]);
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
          println("setMinPulseWidth:",cmd.data[0]);
        }
        if (cmd.cmd.startsWith("Go"))
        {
          // print("Go : ", cmd);
          if (cmd.size > 0)
            go(cmd.data[0]);
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
unsigned long interval = 5000;

// Note the 1 Tick delay, this is need so the watchdog doesn't get confused
void Task2code(void *pvParameters)
{
  Serial.println("Start Task2code");

  while (1)
  {
    try
    {


 unsigned long currentMillis = millis();
  // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillisWifi >=interval)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillisWifi = currentMillis;
  }

  if(WiFi.status() == WL_CONNECTED)
  {
    if(!client.connected() && (currentMillis - previousMillisServer >=interval))
    {
      client.stop();
      if (client.connect("192.168.137.1", 20240))
      {
        Serial.println("Connected to server !");        
      }
      else
      {
        Serial.println("Connection to server failed");
      }      
      previousMillisServer = currentMillis;
    }
  }


#ifdef NO_WIFI
      ESP32_Helper::UpdateSerial();
#endif

#ifdef WITH_WIFI
      ESP32_Helper::UpdateSocket();
#endif

    }
    catch (std::exception const &e)
    {
      Serial.print("error : ");
      Serial.println(e.what());
    }
    vTaskDelay(1);
  }
}