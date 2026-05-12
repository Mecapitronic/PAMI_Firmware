#include "main.h"
#include <esp_task_wdt.h>

using namespace std;
using namespace Printer;
using namespace Hardware_Config;
using namespace ServoAX12;

Adafruit_INA219 ina219;

void setup()
{
  ESP32_Helper::Initialisation();
  println("PAMI Firmware");

  Hardware::Initialisation();
  Power::EnablePower();
  
  Motion::Initialisation();

  // Valeurs par défaut : { ax12Id, {positions[0]=min ... positions[n-1]=max}, count }
  // Modifiables via commande : AX12Config:<nom>:<field>:<valeur>  (field: id|cnt|p0..p9)
  // Stockées en NVS, persistantes au redémarrage
  AddServo(ServoID::VL53, "VL53", ServoConfig(1, std::array<int32_t, MAX_SERVO_POSITIONS>{0, 100, 200, 290}, 4));
  ServoConfig servoConfig;
  servoConfig.ax12Id = 2;
  servoConfig.AddPosition(0, ServoPosition::Min);   // min
  servoConfig.AddPosition(90, ServoPosition::Pos1);  // pos1
  // We do not have to set all positions, only the ones we want to use. The count will be automatically updated to reflect the number of valid positions.
  AddServo(ServoID::Bras, "Bras", servoConfig);

  TaskThread(TaskMatch, "TaskMatch", 20000, 15, 0);
  TaskThread(TaskTeleplot, "TaskTeleplot", 5000, 1, 0);
  TaskThread(TaskHandleCommand, "TaskHandleCommand", 10000, 15, 0);
}

void loop()
{
  // HACK Vérifier qu'on n'utilise pas les serialEvent !!!
  // C:\Users\xxx\.platformio\packages\framework-arduinoespressif32\cores\esp32\main.cpp
  // https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/main.cpp
  vTaskDelete(NULL); // Supprime immédiatement le task Arduino "loop"
}

void TaskMatch(void *pvParameters)
{
  println("Start TaskMatch");
  Chrono chrono("MainMatch", 1000);
  while (1)
  {
    chrono.Start();
    try
    {
      if(Match::matchState == Match::State::MATCH_BOOT)
      {
      }

      // En attente de retrait de la tirette pour démarrer le match
      if (Match::matchState == Match::State::MATCH_WAIT)
      {

        // Start Position
        // Save Y position and orientation

        Motion::SetCurrentY(0);
        Motion::SetCurrentX(Motion::centerPositionMm);
        Motion::SetCurrentRot(0);

        //   if (numPami == 0)
        //     setCurrentY(1924);
        //   else if (numPami == 1)
        //     setCurrentY(1817);
        //   else if (numPami == 2)
        //     setCurrentY(1710);
        //   else if (numPami == 3)
        //     setCurrentY(1603);
        //   else
        //     println("ERROR robot number");

        // if (team == Team::TEAM_YELLOW)
        // {
        //   setCurrentX(Motion::centerPositionMm);
        //   setCurrentRot(0);
        // }
        // else
        // {
        //   setCurrentX(3000-Motion::centerPositionMm);
        //   setCurrentRot(180);
        // }
      }

      // Match en cours
      if (Match::matchState == Match::State::MATCH_RUN)
      {
        /*
        int lastMatchTime = 0;
        while(Match::getMatchTimeMs() < Match::time_start_match && IHM::switchMode == 1)
        {
          // Countdown to start
          if (lastMatchTime != (int)(Match::getMatchTimeSec()))
          {
              println("Match Time : %i", (int)(Match::getMatchTimeSec()));
              lastMatchTime = (int)(Match::getMatchTimeSec());
          }
          vTaskDelay(1);
        }*/

        println("-------");
        println("Start !");

        if (IHM::switchMode == 1)
        {
          println("Mode Match !");
          long speed = 0;
          long accel = 0;
          // Motion::setOpponentChecking(true);

          speed = micros() % (int)(Motion::maxSpeed * 3 / 4) + (int)(Motion::maxSpeed * 1 / 4);
          accel = micros() % (int)(Motion::maxAcceleration * 3 / 4) + (int)(Motion::maxAcceleration * 1 / 4);
          println("speed : %i", speed);
          println("accel : %i", accel);
          Motion::SetMaxSpeed(speed);
          Motion::SetAcceleration(accel);

          Motion::Go(800);

          speed = micros() % (int)(Motion::maxSpeed * 3 / 4) + (int)(Motion::maxSpeed * 1 / 4);
          accel = micros() % (int)(Motion::maxAcceleration * 3 / 4) + (int)(Motion::maxAcceleration * 1 / 4);
          println("speed : %i", speed);
          println("accel : %i", accel);
          Motion::SetMaxSpeed(speed);
          Motion::SetAcceleration(accel);

          Motion::Turn(180);

          speed = micros() % (int)(Motion::maxSpeed * 3 / 4) + (int)(Motion::maxSpeed * 1 / 4);
          accel = micros() % (int)(Motion::maxAcceleration * 3 / 4) + (int)(Motion::maxAcceleration * 1 / 4);
          println("speed : %i", speed);
          println("accel : %i", accel);
          Motion::SetMaxSpeed(speed);
          Motion::SetAcceleration(accel);

          Motion::Go(1000);
        }
        else
        {
          println("Mode Test !");

          long speed = 0;
          long accel = 0;
          // Motion::setOpponentChecking(true);

          speed = Motion::maxSpeed;        // micros()%(int)(Motion::maxSpeed*3/4)+(int)(Motion::maxSpeed*1/4);
          accel = Motion::maxAcceleration; // micros()%(int)(Motion::maxAcceleration*3/4)+(int)(Motion::maxAcceleration*1/4);
          println("speed : %i", speed);
          println("accel : %i", accel);
          Motion::SetMaxSpeed(speed);
          Motion::SetAcceleration(accel);

          Motion::Turn(45);
          Motion::Turn(-90);
          Motion::Turn(45);
          Motion::Turn(-360);
        }

        println("Stop !");
        println("------");

        /*
        if (numPami == 0)
        {
          Motion::SetOpponentChecking(true);
          if (team == Team::TEAM_YELLOW)
          {
            Motion::GoTo(647, 1924);
            Motion::SetMaxSpeed(Motion::maxSpeed/3);
            Motion::SetAcceleration(Motion::maxAcceleration/3);
            Motion::GoTo(1250, 1924);
            Motion::SetMaxSpeed(Motion::maxSpeed/2);
            Motion::SetAcceleration(Motion::maxAcceleration/2);
            Motion::TurnTo(1250, 1580);
            Motion::Go(-100);
            Motion::SetCurrentY(2000-Motion::centerPositionMm);
            Motion::SetMaxSpeed(Motion::maxSpeed);
            Motion::SetAcceleration(Motion::maxAcceleration);
            Motion::GoTo(1250, 1580);
          }
          else
          {
            Motion::GoTo(3000 - 647, 1924);
            Motion::SetMaxSpeed(Motion::maxSpeed/3);
            Motion::SetAcceleration(Motion::maxAcceleration/3);
            Motion::GoTo(3000 - 1250, 1924);
            Motion::SetMaxSpeed(Motion::maxSpeed/2);
            Motion::SetAcceleration(Motion::maxAcceleration/2);
            Motion::TurnTo(3000 - 1250, 1580);
            Motion::Go(-100);
            Motion::SetCurrentY(2000-Motion::centerPositionMm);
            Motion::SetMaxSpeed(Motion::maxSpeed);
            Motion::SetAcceleration(Motion::maxAcceleration);
            Motion::GoTo(3000 - 1250, 1580);
          }
        }
        else if (numPami == 1)
        {
          delay(3000);
          if (team == Team::TEAM_YELLOW)
          {
            Motion::GoTo(350, 1817);
            Motion::TurnTo(750, 1500);
            Motion::GoTo(750, 1500);
          }
          else
          {
            Motion::GoTo(3000 - 350, 1817);
            Motion::TurnTo(3000 - 750, 1500);
            Motion::GoTo(3000 - 750, 1500);
          }
        }
        else if (numPami == 2)
        {
          if (team == Team::TEAM_YELLOW)
          {
            Motion::GoTo(375, 1710);
            Motion::TurnTo(1500, 1250);
            Motion::SetMaxSpeed(Motion::maxSpeed/2);
            Motion::SetAcceleration(Motion::maxAcceleration/2);
            Motion::GoTo(1500, 1250);
          }
          else
          {
            Motion::GoTo(3000 - 375, 1710);
            Motion::TurnTo(3000 - 1500, 1250);
            Motion::SetMaxSpeed(Motion::maxSpeed/2);
            Motion::SetAcceleration(Motion::maxAcceleration/2);
            Motion::GoTo(3000 - 1500, 1250);
          }
        }
        else if (numPami == 3)
        {
          if (team == Team::TEAM_YELLOW)
          {
            Motion::GoTo(550, 1603);
            Motion::TurnTo(1500, 1250);
          }
          else
          {
            Motion::GoTo(3000 - 550, 1603);
            Motion::TurnTo(3000 - 1500, 1250);
          }
        }
        */

        // Fin des actions
        Match::matchState = Match::State::MATCH_STOP;
      }

      // Arrêt des PAMI
      if (Match::matchState == Match::State::MATCH_STOP)
      {
        // Match::matchState = Match::State::MATCH_NONE;
        // Wait for end of match
      }

      // Fin du match
      if (Match::matchState == Match::State::MATCH_END)
      {
        // Wait for reset
        if (IHM::switchMode == 0 && IHM::tirettePresent == 0)
          Match::matchState = Match::State::MATCH_BOOT;
      }
    }
    catch (std::exception const &e)
    {
      printError(e.what());
    }
    if (chrono.Check())
    {
      printChrono(chrono);
    }
    vTaskDelay(10);
  }
}

Pose MapBoundaries[] = {{0, 0, 0}, {0, 2000, 0}, {3000, 2000, 0}, {3000, 0, 0}};

void TaskTeleplot(void *pvParameters)
{
  int lastMatchTime = 0;
  println("Start TaskTeleplot");
  Timeout teleplotTO;
  teleplotTO.Start(100);
  Chrono chrono("Teleplot", 1000);

  while (true)
  {
    chrono.Start();
    try
    {
      if (teleplotTO.IsTimeOut())
      {
        const Pose pose = Motion::GetCurrentPose();
        Printer::teleplot("pos", pose);
        Printer::teleplot("ang", pose.h);
        //Printer::teleplot("speed_D", Motion::motor_D.speed());
        //Printer::teleplot("speed_G", Motion::motor_G.speed());

        // Countdown
        if (lastMatchTime != (int)(Match::getMatchTimeSec()) && Match::matchState != Match::State::MATCH_BOOT)
        {
          println("Match Time : %i", (int)(Match::getMatchTimeSec()));
          lastMatchTime = (int)(Match::getMatchTimeSec());
        }
      }
    }
    catch (const std::exception &e)
    {
      printError(e.what());
    }
    if (chrono.Check())
    {
      printChrono(chrono);
    }
    vTaskDelay(10);
  }
}

void TaskHandleCommand(void *pvParameters)
{
  println("Start TaskHandleCommand");
  Chrono chrono("HandleCommand", 1000);

  while (true)
  {
    chrono.Start();
    try
    {
      // Check if we get commands from operator via debug serial
      if (ESP32_Helper::HasWaitingCommand())
      {
        Command cmd = ESP32_Helper::GetCommand();

        if (cmd.cmdStartsWith("Pos"))
        {
          println("Pos x=%d y=%d h=%d", (int)Motion::GetCurrentPose().x, (int)Motion::GetCurrentPose().y, (int)Motion::GetCurrentPose().h);
        }
        if (cmd.cmdStartsWith("Speed"))
        {
          // print("Speed : ", cmd);
          if (cmd.size > 0)
          {
            Motion::SetMaxSpeed(cmd.data[0]);
            //preferences.putInt("Speed",cmd.data[0]);
            println("Speed : %f", Motion::GetMaxSpeed());
          }
          println("Motor D speed: %f", Motion::motor_D.maxSpeed());
          println("Motor G speed: %f", Motion::motor_G.maxSpeed());
        }
        if (cmd.cmdStartsWith("Accel"))
        {
          // print("Accel : ", cmd);
          if (cmd.size > 0)
          {
            Motion::SetAcceleration(cmd.data[0]);
            //preferences.putInt("Accel",cmd.data[0]);
            println("Accel : %f", Motion::GetAcceleration());
          }
          println("Motor D accel: %f", Motion::motor_D.getAcceleration());
          println("Motor G accel: %f", Motion::motor_G.getAcceleration());
        }
        if (cmd.cmdStartsWith("Go"))
        {
          // print("Go : ", cmd);
          if (cmd.size == 1)
            Motion::Go(cmd.data[0]);
          if (cmd.size == 2)
            Motion::GoTo(cmd.data[0], cmd.data[1]);
          if (cmd.size == 3)
            Motion::GoTo(cmd.data[0], cmd.data[1], cmd.data[2]);
        }
        if (cmd.cmdStartsWith("Turn"))
        {
          // print("Turn : ", cmd);
          if (cmd.size > 0)
            Motion::Turn(cmd.data[0]);
        }
        if (cmd.cmdStartsWith("Motor"))
        {
          println("Motor D:");
          println("speed: %f", Motion::motor_D.speed());
          println("acceleration: %f", Motion::motor_D.getAcceleration());
          println("distanceToGo: %i", (int)Motion::motor_D.distanceToGo());
          println("targetPosition: %i", (int)Motion::motor_D.targetPosition());
          println("currentPosition: %i", (int)Motion::motor_D.currentPosition());
          // println("computeNewSpeed:",(long)Motion::motor_D.computeNewSpeed());
          println("-----");
          println("Motor G:");
          println("speed: %f", Motion::motor_G.speed());
          println("acceleration: %f", Motion::motor_G.getAcceleration());
          println("distanceToGo: %i", (int)Motion::motor_G.distanceToGo());
          println("targetPosition: %i", (int)Motion::motor_G.targetPosition());
          println("currentPosition: %i", (int)Motion::motor_G.currentPosition());
          // println("computeNewSpeed:",(long)Motion::motor_G.computeNewSpeed());
          println("-----");
        }
        if (cmd.cmdStartsWith("Blink"))
        {
          //Blink:0
          //Blink:1
          if (cmd.size > 0)
          {
            IHM::useBlink = cmd.data[0];
            println("Blink : %i", IHM::useBlink);
          }
        }
        if (cmd.cmdStartsWith("RGB"))
        {
          //RGB:0:255:0
          if (cmd.size == 3)
          {
            // IHM::led[0].setRGB(cmd.data[0], cmd.data[1], cmd.data[2]);
            // FastLED.show();
            // print("RGB : ", led[0].red);
            // print(" ", led[0].green);
            // println(" ", led[0].blue);
          }
        }
        if (cmd.cmdStartsWith("HSV"))
        {
          //HSV:0:255:255
          //  if (cmd.size == 1)
          //  {
          //    led[0].setHue(cmd.data[0]);
          //  }
          //  if (cmd.size == 3)
          //  {
          //    led[0].setHSV(cmd.data[0], cmd.data[1], cmd.data[2]);
          //  }
          //  FastLED.show();
          //  print("HSV : ", led[0].red);
          //  print(" ", led[0].green);
          //  println(" ", led[0].blue);
        }
      }
    }
    catch (std::exception const &e)
    {
      print("error : ");
      println(e.what());
    }
    if (chrono.Check())
    {
      printChrono(chrono);
    }
    vTaskDelay(10);
  }
}
