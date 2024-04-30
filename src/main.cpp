#include "main.h"

Preferences preferences;

void setup()
{
  Serial.end();
  Serial.begin(921600);
  delay(1000);
  Serial.println("PAMI Firmware");

  Serial.print("AP MAC : ");
  Serial.println(WiFi.softAPmacAddress());
  Serial.print("Wifi MAC : ");
  Serial.println(WiFi.macAddress());

  int num_pami=0;
  for (size_t i = 0; i < max_pami; i++)
  {
    if (WiFi.softAPmacAddress() == ap_mac_pami[i] || WiFi.macAddress() == wifi_mac_pami[i])
    {
      num_pami = i+1;
      Serial.print("Num PAMI : ");
      Serial.println(num_pami);
      //WiFi.softAP(ap_ssid + "_" + num_pami, ap_password);      
      break;
    }
  }

#ifdef WITH_WIFI
  // Begin Access Point
  //WiFi.mode(WIFI_AP_STA);
  WiFi.mode(WIFI_STA);

/*
  Serial.print("AP IP address : ");
  Serial.println(WiFi.softAPIP());
  Serial.print("AP SSID : ");
  Serial.println(WiFi.softAPSSID());
  Serial.print("AP MAC : ");
  Serial.println(WiFi.softAPmacAddress());
*/

// Set your Static IP address
IPAddress local_IP(192, 168, 137, 100+num_pami);
// Set your Gateway IP address
IPAddress gateway(192, 168, 137, 1);

IPAddress subnet(255, 255, 255, 0);
//IPAddress primaryDNS(8, 8, 8, 8);   //optional
//IPAddress secondaryDNS(8, 8, 4, 4); //optional

  // Configures static IP address
  if (!WiFi.config(local_IP, gateway, subnet))//, primaryDNS, secondaryDNS))
  {
    Serial.println("STA Failed to configure");
  }
  
  // delete old config
  WiFi.disconnect(true);
  
  // time to disconnect and turn Wi-Fi radio off
  delay(1000);

  // Events callback (to reconnect)
  WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  // Begin WiFi
  WiFi.begin(wifi_ssid, wifi_password);

  // time to connect
  delay(1000);

  // Test if connected
  //if (WiFi.waitForConnectResult() != WL_CONNECTED)
  //{
  //  Serial.println("Still connecting ...");
  //}

  // WebSerial is accessible at "<IP Address>/webserial" in browser
  WebSerial.begin(&server);
  /* Attach Message Callback */
  WebSerial.msgCallback(recvMsg);
  server.begin();
  /*
      WiFi.mode(WIFI_AP);
      WiFi.softAP(SSID, PASSWORD);

      IPAddress IP = WiFi.softAPIP();
      Serial.print("AP IP address: ");
      Serial.println(IP);
  */

  OTA.begin(); // Setup settings
#endif

  ESP32_Helper::ESP32_Helper();

  // myQueue = xQueueCreate(queueSize, sizeof(PolarPoint));
  myQueue = xQueueCreate(queueSize, sizeof(uint8_t));

  if (myQueue == NULL)
  {
    Serial.println("Error creating the queue");
  }

  // Sets the Sensor pins as Inputs
  pinMode(DETECT_1, INPUT);
  pinMode(DETECT_2, INPUT);

  readRobotNumber();

  initMotion();

  enableMotors();

  // waitStart();

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
  delay(1000);
}

// Note the 1 Tick delay, this is need  so the watchdog doesn't get confused
void Task1code(void *pvParameters)
{
  Serial.println("Start Task1code");

  setMaxSpeed(5000);
  setAcceleration(500);

  while (1)
  {
    try
    {
      /*if (Serial.available() > 0)
      {
          uint32_t tmpInt = Serial.read();
          xQueueSend(myQueue, &tmpInt, 0);
      }*/

      // updateMatchTime();
      // match();

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
      /*
              setMaxSpeed(10000);
              setAcceleration(500);
              go(100*2);
              turn(180*3);
              go(100*2);
              turn(-180*3);
      */

      // Serial.println("End of cycle");
    }
    catch (std::exception const &e)
    {
      Serial.print("error : ");
      Serial.println(e.what());
    }
    vTaskDelay(1);
  }
}

// Note the 1 Tick delay, this is need so the watchdog doesn't get confused
void Task2code(void *pvParameters)
{
  Serial.println("Start Task2code");

  while (1)
  {
    try
    {
      if (uxQueueMessagesWaiting(myQueue) > 0)
      {
        int integer;
        if (xQueueReceive(myQueue, &integer, portTICK_PERIOD_MS * 0))
        {
          Serial.println("integer : " + String(integer));
        }
      }
      if (motor_D.isRunning() || motor_G.isRunning())
      {
        enableMotors();
        updateMotors();
      }
      else
      {
        disableMotors();
      }
#ifdef WITH_WIFI
      OTA.handle();
#endif
#ifdef NO_WIFI
      ESP32_Helper::UpdateSerial();
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

void waitStart()
{
  // Attendre que la tirette n'est soit plus présente
  // infoLCD("Remove Tirette");
  // while(getTirette()) {
  // delay(500);
  // checkColorTeam();
  //}
  // Attendre que la tirette soit insérée
  // infoLCD("Insert Tirette");
  // while(!getTirette()) {
  //  delay(500);
  //  checkColorTeam();
  //}
  // Datum position du PAMI
  delay(2000);
  datumPosition(getRobotNumber(), getTeamColor());
  // setRobotState(READY);
  // infoLCD("Robot Ready");
  delay(2000);
  // Attendre que la tirette soit bien insérée pour éviter les faux-départs
  // infoLCD("Insert Tirette");
  // while(!getTirette()) delay(500);
  // Attendre que la tirette soit retirée pour débuter le match
  // infoLCD("Wait Start");
  // while(getTirette()) delay(250);
  // Le match commence
  // setRobotState(MATCH_STARTED);
  // infoLCD("Go Match !");
  // Démarrage du compteur !
  startMatch();
}

void datumPosition(int robotNumber, int teamColor)
{

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

    if (robotNumber == 1)
      goTo(1120, 80, 270); // Go to safe position
    else if (robotNumber == 2)
      goTo(1120 + 130, 80, 270); // Go to safe position
    else if (robotNumber == 3)
      goTo(1120 + 260, 80, 270); // Go to safe position
    else
      Serial.println("ERROR robot number");
  }
  else if (teamColor == TEAM_YELLOW)
  {
    goTo(0, 80, 180);
    go(-100);
    // SaveX position and orientation
    setCurrentX(1950 - CENTER_POSITION_MM);
    setCurrentRot(180);

    if (robotNumber == 1)
      goTo(1880, 80, 270); // Go to safe position
    else if (robotNumber == 2)
      goTo(1880 - 130, 80, 270); // Go to safe position
    else if (robotNumber == 3)
      goTo(1880 - 260, 80, 270); // Go to safe position
    else
      Serial.println("ERROR robot number");
  }

  setMaxSpeed(MAX_SPEED);
  setAcceleration(MAX_ACCELERATION);
}

void match()
{
  if (getMatchState() == PAMI_RUN)
  {
    enableMotors();
    strategiePAMI();
    setMatchState(PAMI_STOP);
  }
  else if (getMatchState() == PAMI_STOP)
  {
    disableMotors(); // Desactive les moteurs
    Serial.println("Match STOP");
    while (1)
    {
      vTaskDelay(1);
    } // Fin de match
  }
  else
  {
    disableMotors(); // Desactive les moteurs
  }
}

void strategiePAMI()
{

  setOpponentChecking(true);
  if (getRobotNumber() == 1)
  {
    if (getTeamColor() == TEAM_BLUE)
    {
      goTo(750, 180);
      setOpponentChecking(false);
      goTo(750, 0);
      // antennasDown();
    }
    else
    {
      goTo(3000 - 750, 180);
      setOpponentChecking(false);
      goTo(3000 - 750, 0);
      // antennasDown();
    }
  }
  else if (getRobotNumber() == 2)
  {
    if (getTeamColor() == TEAM_BLUE)
    {
      goTo(1200, 300);
      goTo(600, 300);
      setOpponentChecking(false);
      goTo(400, 300);
      // antennasDown();
    }
    else
    {
      goTo(3000 - 1200, 300);
      goTo(3000 - 600, 300);
      setOpponentChecking(false);
      goTo(3000 - 400, 300);
      // antennasDown();
    }
  }
  else if (getRobotNumber() == 3)
  {
    if (getTeamColor() == TEAM_BLUE)
    {
      goTo(1350, 450);
      goTo(400, 550);
      setOpponentChecking(false);
      goTo(0, 550);
      // antennasDown();
    }
    else
    {
      goTo(3000 - 1350, 450);
      goTo(3000 - 400, 550);
      setOpponentChecking(false);
      goTo(3000 - 0, 550);
      // antennasDown();
    }
  }
}
byte robotNumber = 1;
bool team = TEAM_BLUE;

byte readRobotNumber()
{
  // Read robotNumber
  bool bit1 = 0; //! digitalRead(BOT_BIT_1);
  bool bit2 = 0; // !digitalRead(BOT_BIT_2);
  // robotNumber = (bit2 << 1) | bit1;
  return robotNumber;
}

byte getRobotNumber()
{
  return robotNumber;
}

bool getTeamColor()
{
  return team;
}

/* Message callback of WebSerial */
void recvMsg(uint8_t *data, size_t len)
{
  // WebSerial.println("Received Data...");
  // String d = "";
  for (int i = 0; i < len; i++)
  {
    // d += char(data[i]);
    ESP32_Helper::UpdateSerial(char(data[i]));
  }
  ESP32_Helper::UpdateSerial('\n');
  // WebSerial.println(d);
}


void WiFiStationConnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Connected to " + WiFi.SSID() + " successfully!");
}

void WiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  Serial.println("Disconnected from WiFi access point");
  Serial.print("WiFi lost connection. Reason: ");
  Serial.println((wifi_err_reason_t)info.wifi_sta_disconnected.reason);
  Serial.println("Trying to Reconnect");
  // Begin WiFi
  WiFi.begin(wifi_ssid, wifi_password);
}
