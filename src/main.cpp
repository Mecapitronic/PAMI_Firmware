#include "main.h"

using namespace Printer;
using namespace std;

Preferences preferences;

void setup()
{  
  ESP32_Helper::Initialisation();
  delay(2000);
  println("PAMI Firmware");

  InitIHM();
  initSensor();
  initMotion();
  
  if (checkSensor())
  {
    println("Opponent detected");
  }
  else
  {
    println("No opponent detected");
  }

  delay(2000);

  int speedPref = preferences.getInt("Speed",0);
  int accelPref = preferences.getInt("Accel",0);
    
  if(speedPref != 0)
  {
    setMaxSpeed(speedPref);
    println("Speed : ", speedPref);
  }
  if(accelPref != 0)
  {
    setAcceleration(accelPref);
    println("Accel : ", accelPref);
  }

  /* Task function. */
  /* name of task. */
  /* Stack size of task */
  /* parameter of the task */
  /* priority of the task */
  /* Task handle to keep track of created task */
  /* pin task to core 0 */
  xTaskCreatePinnedToCore(TaskMatch, "TaskMatch", 20000, NULL, 10, &Task1, 0);
  xTaskCreatePinnedToCore(TaskSerial, "TaskSerial", 20000, NULL, 5, &Task2, 1);
}


void loop()
{
  updateMatch();
  UpdateBAU();
  if (matchState != State::MATCH_STOP && matchState != State::MATCH_END && (motor_D.isRunning() || motor_G.isRunning()))
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
void TaskMatch(void *pvParameters)
{
  println("Start TaskMatch");

  while (1)
  {
    try
    {
      // Attente du démarrage du match par la tirette
      if (matchState == State::MATCH_WAIT)
      {
        UpdateHMI();

        // Lecture do codage du numéro de PAMI
        int numPamiTmp = ReadNumPami();
        if (numPamiTmp != numPami)
        {
          numPami = numPamiTmp;
          println("N° PAMI : ", numPami);
          Wifi_Helper::SetLocalIP("192.168.137." + String(100 + numPami + 1));
        }
        // Start Position
        // Save Y position and orientation
        setCurrentY(CENTER_POSITION_MM);
        setCurrentRot(90);
        if (team == Team::TEAM_YELLOW)
        {
          if (numPami == 0)
            setCurrentX(1102);
          else if (numPami == 1)
            setCurrentX(1216);
          else if (numPami == 2)
            setCurrentX(1330);
          else if (numPami == 3)
            setCurrentX(1444);
          else
            println("ERROR robot number");
        }
        else
        {
          if (numPami == 0)
            setCurrentX(1898);
          else if (numPami == 1)
            setCurrentX(1784);
          else if (numPami == 2)
            setCurrentX(1670);
          else if (numPami == 3)
            setCurrentX(1556);
          else
            println("ERROR robot number");
        }
      }

      // Match en cours
      if (matchState == State::MATCH_BEGIN)
      {
        // Countdown
        if(lastMatchTime != (int)(getMatchTime()/1000))
        {
          println("Match Time : ", (int)(getMatchTime()/1000));
          lastMatchTime=(int)(getMatchTime()/1000);
        }
      }

      // Démarrage des PAMI
      if (matchState == State::MATCH_RUN)
      {
        setOpponentChecking(true);
        if (numPami == 0)
        {
          if (team == Team::TEAM_YELLOW)
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
        else if (numPami == 1)
        {
          if (team == Team::TEAM_YELLOW)
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
        else if (numPami == 2)
        {
          if (team == Team::TEAM_YELLOW)
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
        else if (numPami == 3)
        {
          if (team == Team::TEAM_YELLOW)
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
        stopMatch();
      }

      // Arrêt des PAMI
      if (matchState == State::MATCH_STOP)
      {
        // Wait for end of match
      }
      
      // Fin du match
     if (matchState == State::MATCH_END)
     {
        useBlink = false;
     }
    }
    catch (std::exception const &e)
    {
      print("error : ");
      println(e.what());
    }
    vTaskDelay(1);
  }
}

Pose MapBoundaries[] = {{0, 0, 0}, {0, 2000, 0}, {3000, 2000, 0}, {3000, 0, 0}};
Timeout teleplotTO;
// Note the 1 Tick delay, this is need so the watchdog doesn't get confused
void TaskSerial(void *pvParameters)
{
  println("Start TaskSerial");
  teleplotTO.Start(500);

  while (1)
  {
    try
    {
      if (teleplotTO.IsTimeOut())
      {
        Printer::teleplot("pos", getCurrentPose());
        Printer::teleplot("ang", (int)(getCurrentPose().h));
        //Printer::teleplot("mapBoundaries", MapBoundaries[0]);
        //Printer::teleplot("mapBoundaries", MapBoundaries[1]);
        //Printer::teleplot("mapBoundaries", MapBoundaries[2]);
        //Printer::teleplot("mapBoundaries", MapBoundaries[3]);
      }
      
      // Check if we get commands from operator via debug serial
      if (ESP32_Helper::HasWaitingCommand())
      {
        Command cmd = ESP32_Helper::GetCommand();

        if(cmd.cmd.startsWith("Pos"))
        {
          print("Pos : x=", getCurrentPose().x);
          print("  y=", getCurrentPose().y);
          print("  h=", getCurrentPose().h);
          println();
        }
        if (cmd.cmd.startsWith("Speed"))
        {
          // print("Speed : ", cmd);
          if (cmd.size > 0)
          {
            setMaxSpeed(cmd.data[0]);
            preferences.putInt("Speed",cmd.data[0]);
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
            preferences.putInt("Accel",cmd.data[0]);
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
          //Blink:0
          //Blink:1
          if (cmd.size > 0)
          {            
            useBlink = cmd.data[0];
            println("Blink : ", useBlink);
          }
        }
        if (cmd.cmd.startsWith("RGB"))
        {
          //RGB:0:255:0
          if (cmd.size == 3)
          {
            led[0].setRGB(cmd.data[0], cmd.data[1], cmd.data[2]);
            FastLED.show();
            print("RGB : ", led[0].red);
            print(" ", led[0].green);
            println(" ", led[0].blue);
          }
        }
        if (cmd.cmd.startsWith("HSV"))
        {
          //HSV:0:255:255
          if (cmd.size == 1)
          {
            led[0].setHue(cmd.data[0]);
          }
          if (cmd.size == 3)
          {
            led[0].setHSV(cmd.data[0], cmd.data[1], cmd.data[2]);
          }
          FastLED.show();
          print("HSV : ", led[0].red);
          print(" ", led[0].green);
          println(" ", led[0].blue);
        }
      }
    }
    catch (std::exception const &e)
    {
      print("error : ");
      println(e.what());
    }
    vTaskDelay(1);
    // vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
