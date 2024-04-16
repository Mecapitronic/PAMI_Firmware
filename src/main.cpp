#include "main.h"

void setup()
{
    // myQueue = xQueueCreate(queueSize, sizeof(PolarPoint));
    myQueue = xQueueCreate(queueSize, sizeof(uint8_t));

    if (myQueue == NULL)
    {
        Serial.println("Error creating the queue");
    }

    Serial.end();
    Serial.begin(921600);
    delay(1000);
    Serial.println("PAMI Firmware");
    
    // Sets the Sensor pins as Inputs
    pinMode(DETECT_1, INPUT);
    pinMode(DETECT_2, INPUT);

    readRobotNumber();

    initMotion();
    
    enableMotors();
    
    waitStart();

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

// Note the 1 Tick delay, this is need so the watchdog doesn't get confused
void Task1code(void *pvParameters)
{
    Serial.println("Start Task1code");

    while (1)
    {
        try
        {
            if (Serial.available() > 0)
            {
                uint32_t tmpInt = Serial.read();
                xQueueSend(myQueue, &tmpInt, 0);
            }

        updateMatchTime();
        match();

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
        }
        catch (std::exception const &e)
        {
            Serial.print("error : ");
            Serial.println(e.what());
        }
        vTaskDelay(1);
    }
}

void waitStart(){
  // Attendre que la tirette n'est soit plus présente
  //infoLCD("Remove Tirette");
  //while(getTirette()) {
    //delay(500); 
    //checkColorTeam();
  //}
  // Attendre que la tirette soit insérée
  //infoLCD("Insert Tirette");
  //while(!getTirette()) {
  //  delay(500); 
  //  checkColorTeam();
  //}
  // Datum position du PAMI
  delay(2000);
  datumPosition(getRobotNumber(), getTeamColor());
  //setRobotState(READY);
  //infoLCD("Robot Ready");
  delay(2000);
  // Attendre que la tirette soit bien insérée pour éviter les faux-départs
  //infoLCD("Insert Tirette");
  //while(!getTirette()) delay(500);
  // Attendre que la tirette soit retirée pour débuter le match
  //infoLCD("Wait Start");
  //while(getTirette()) delay(250);
  // Le match commence
  //setRobotState(MATCH_STARTED);
  //infoLCD("Go Match !");
  // Démarrage du compteur !
  startMatch();
}

void datumPosition(int robotNumber, int teamColor){

  // Datum at low Speed
  setMaxSpeed(DATUM_SPEED);
  setAcceleration(DATUM_ACCELERATION);
  // Datum Y
  go(-100);
  // Save Y position and orientation
  setCurrentY(CENTER_POSITION_MM);
  setCurrentRot(270);

  if (teamColor == TEAM_BLUE){
    
    // Orientate robot
    goTo(0,80,0);
    go(-100);
    // SaveX position and orientation
    setCurrentX(1050+CENTER_POSITION_MM);
    setCurrentRot(0);

    if(robotNumber == 1) goTo(1120,80,270); // Go to safe position
    else if(robotNumber == 2) goTo(1120+130,80,270); // Go to safe position
    else if(robotNumber == 3) goTo(1120+260,80,270); // Go to safe position
    else Serial.println("ERROR robot number");
  }
  else if (teamColor == TEAM_YELLOW){
    goTo(0,80,180);
    go(-100);
    // SaveX position and orientation
    setCurrentX(1950-CENTER_POSITION_MM);
    setCurrentRot(180);

    if(robotNumber == 1) goTo(1880,80,270); // Go to safe position
    else if(robotNumber == 2) goTo(1880-130,80,270); // Go to safe position
    else if(robotNumber == 3) goTo(1880-260,80,270); // Go to safe position
    else Serial.println("ERROR robot number");
  }

  setMaxSpeed(MAX_SPEED);
  setAcceleration(MAX_ACCELERATION);
}

void match(){
  if(getMatchState() == PAMI_RUN){
    enableMotors();
    strategiePAMI();
    setMatchState(PAMI_STOP);
  }
  else if (getMatchState() == PAMI_STOP){
    disableMotors(); // Desactive les moteurs
    //while(1); // Fin de match
  }
  else {
    disableMotors(); // Desactive les moteurs
  }  
}

void strategiePAMI(){

  setOpponentChecking(true);
  if(getRobotNumber() == 1){
    if(getTeamColor() == TEAM_BLUE){
      goTo(750,180);
      setOpponentChecking(false);
      goTo(750,0);
      //antennasDown();
    }
    else{
      goTo(3000-750,180);
      setOpponentChecking(false);
      goTo(3000-750,0);
      //antennasDown();
    }
  }
  else if(getRobotNumber() == 2){
    if(getTeamColor() == TEAM_BLUE){
      goTo(1200,300);
      goTo(600,300);
      setOpponentChecking(false);
      goTo(400,300);
      //antennasDown();
    }
    else{
      goTo(3000-1200,300);
      goTo(3000-600,300);
      setOpponentChecking(false);
      goTo(3000-400,300);
      //antennasDown();
    }
  }
  else if(getRobotNumber() == 3){
    if(getTeamColor() == TEAM_BLUE){
      goTo(1350,450);
      goTo(400,550);
      setOpponentChecking(false);
      goTo(0,550);
      //antennasDown();
    }
    else{
      goTo(3000-1350,450);
      goTo(3000-400,550);
      setOpponentChecking(false);
      goTo(3000-0,550);
      //antennasDown();
    }
  }
}
byte robotNumber =1;
bool team = TEAM_BLUE;

byte readRobotNumber(){
  // Read robotNumber
  bool bit1 = 0; //!digitalRead(BOT_BIT_1);
  bool bit2 = 0; // !digitalRead(BOT_BIT_2);
  //robotNumber = (bit2 << 1) | bit1;
  return robotNumber;
}

byte getRobotNumber(){
  return robotNumber;
}

bool getTeamColor(){
  return team;
}
