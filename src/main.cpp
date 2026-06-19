#include "main.h"

using namespace std;
using namespace Printer;
using namespace Hardware_Config;
using namespace ServoAX12;
using namespace PamiConfiguration;

int numPami = -1;

void setup()
{
  ESP32_Helper::Initialisation();
  println("PAMI Firmware");

  Hardware::Initialisation(true);
  Power::EnablePower();

  Motion::Initialisation();
  
  ApplyPamiConfig();

  TaskThread(TaskMatch, "TaskMatch", 20000, 15, 0);
  //TaskThread(TaskTeleplot, "TaskTeleplot", 20000, 1, 0);
  TaskThread(TaskHandleCommand, "TaskHandleCommand", 10000, 15, 0);
}

void loop()
{
  // HACK Vérifier qu'on n'utilise pas les serialEvent !!!
  // C:\Users\xxx\.platformio\packages\framework-arduinoespressif32\cores\esp32\main.cpp
  // https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/main.cpp
  vTaskDelete(NULL); // Supprime immédiatement le task Arduino "loop"
}

void ApplyPamiConfig()
{
  if (numPami == -1 || numPami != Match::GetNumPami())
  {
    numPami = Match::GetNumPami();
    const PamiConfig *config = GetPamiConfig(numPami);
    if (config)
    {
      println("Applying PAMI Config for PAMI %d", numPami);
      Motion::SetCurrentRot(-90);
      ApplyStartPose(*config);
      Screen::SetPose(Motion::GetCurrentPose());
      ApplyAx12PamiConfig(*config);
    }
    else
    {
      println("Invalid N° PAMI %d", numPami);
    }
  }
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
      if (Match::matchState == Match::State::MATCH_BOOT)
      {
        ApplyPamiConfig();
        Motion::SetOpponentChecking(false);
        if (Match::getMatchTimeMs() > 10000 && IHM::switchMode == 1)
        {
          static bool pos = false;
          if (!pos)
          {
            ServoAX12::SetServoPosition(ServoID::VL53, ServoPosition::Pos2, 5000);
            while (ServoAX12::IsServoMoving(ServoID::VL53))
            {
              vTaskDelay(100);
            }
            pos = true;
          }
          else
          {
            ServoAX12::SetServoPosition(ServoID::VL53, ServoPosition::Pos1, 5000);
            while (ServoAX12::IsServoMoving(ServoID::VL53))
            {
              vTaskDelay(100);
            }
            pos = false;
          }
        }
      }

      // En attente de retrait de la tirette pour démarrer le match
      if (Match::matchState == Match::State::MATCH_WAIT)
      {
        ServoAX12::SetServoPosition(ServoID::VL53, ServoPosition::Pos1, 5000);
        ServoAX12::SetServoPosition(ServoID::Bras, ServoPosition::Pos1, 5000);
        Motion::SetOpponentChecking(false);
        ApplyPamiConfig();
      }

      // Match en cours
      if (Match::matchState == Match::State::MATCH_RUN)
      {
        println("-------");
        println("Start Match");

        ServoAX12::SetServoPosition(ServoID::VL53, ServoPosition::Pos2);
        ServoAX12::SetServoPosition(ServoID::Bras, ServoPosition::Pos1);

        if(IHM::switchMode == 0)
        {
          Motion::SetOpponentChecking(false);
           VL53L8CX_ResultsData data = ToF_VL53L8CX::getSensorData();

           int turn = 0;
           int distance = 0;
           int column[8] = {0,0,0,0,0,0,0,0};
           int distMax = 300;
           for (size_t i = 0; i < 8; i++)
           {              
              if(data.target_status[i*8] == 5 && data.distance_mm[i*8] < distMax)
              {
                // detect at left, fisrt column
                turn +=3;
                column[7]++;
              }
              if(data.target_status[i*8+1] == 5 && data.distance_mm[i*8+1] < distMax)
              {
                // detect at left, second column
                turn +=2;
                column[6]++;
              }
              if(data.target_status[i*8+2] == 5 && data.distance_mm[i*8+2] < distMax)
              {
                // detect at left, third column
                turn +=1;
                column[5]++;
              }
              
              if(data.target_status[i*8+3] == 5 && data.distance_mm[i*8+3] < distMax)
              {
                // detect at center
                distance += (data.distance_mm[i*8+3]-200)/8/2;
                column[4]++;
              }
              if(data.target_status[i*8+4] == 5 && data.distance_mm[i*8+4] < distMax)
              {
                // detect at center
                distance += (data.distance_mm[i*8+4]-200)/8/2;
                column[3]++;
              }
              
              if(data.target_status[i*8+5] == 5 && data.distance_mm[i*8+5] < distMax)
              {
                // detect at right, third column
                turn -=1;
                column[2]++;
              }              
              if(data.target_status[i*8+6] == 5 && data.distance_mm[i*8+6] < distMax)
              {
                // detect at right, second column
                turn -=2;
                column[1]++;
              }
              if(data.target_status[i*8+7] == 5 && data.distance_mm[i*8+7] < distMax)
              {
                // detect at right, fisrt column
                turn -=3;
                column[0]++;
              }
              
            Screen::SetObstacle(i, column[i]);
           }
          Motion::Turn(turn);
          Motion::Go(distance);

        }
        else
        {
          delay(10000);

          const PamiConfig *config = GetPamiConfig(numPami);
          if (config)
          {
            ExecutePamiMovement(config->initialMove);
          }
          else
          {
            println("Invalid PAMI number %d in MATCH_RUN", numPami);
          }

          println("-------");
          println("Wait For 85 Sec !");
          int lastMatchTime = 0;
          while (Match::getMatchTimeMs() < Match::time_start_match && IHM::switchMode == 1)
          {
            // Countdown to start
            if (lastMatchTime != (int)(Match::getMatchTimeSec()))
            {
              println("Match Time %i", (int)(Match::getMatchTimeSec()));
              lastMatchTime = (int)(Match::getMatchTimeSec());
            }
            vTaskDelay(100);
            if (config)
            {
              if (config->isNinja)
                break;
            }
          }

          println("-------");
          println("Start !");

          Motion::SetMaxSpeed(Motion::maxSpeed);
          Motion::SetAcceleration(Motion::maxAcceleration);

          Motion::SetOpponentChecking(true);

          if (config)
          {
            ExecutePamiMovement(config->matchMove1);
            ExecutePamiMovement(config->matchMove2);
            if (config->isNinja)
            {
              if (IHM::team == IHM::Team::Jaune)
              {
                Motion::Turn(-90);
              }
              else
              {
                Motion::Turn(90);
              }
              Motion::Go(-400);
              Motion::SetCurrentY(1910);
              Motion::SetCurrentRot(-90);
              const float targetX = TeamAwareX(1780);
              Motion::GoTo(targetX, 1640);
              // delay(7000);
              //  if (IHM::team == IHM::Team::Jaune)
              //  {
              //    Motion::Turn(-100);
              //  }
              //  else{
              //    Motion::Turn(-80);
              //  }
              //  Motion::Go(150);
            }
          }
          else
          {
            println("Invalid PAMI number %d in MATCH_RUN second move", numPami);
          }

          println("Stop !");
          println("------");

          // Fin des actions
          Match::matchState = Match::State::MATCH_STOP;
        }
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
        static bool pos = false;
        if (!pos)
        {

          ServoAX12::SetServoPosition(ServoID::Bras, ServoPosition::Pos2);

          ServoAX12::SetServoPosition(ServoID::VL53, ServoPosition::Pos2, 5000);
          while (ServoAX12::IsServoMoving(ServoID::VL53))
          {
            vTaskDelay(100);
          }
          pos = true;
        }
        else
        {
          ServoAX12::SetServoPosition(ServoID::VL53, ServoPosition::Pos1, 5000);
          while (ServoAX12::IsServoMoving(ServoID::VL53))
          {
            vTaskDelay(100);
          }
          pos = false;
        }

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
        // Printer::teleplot("pos", pose);
        // Printer::teleplot("ang", pose.h);

        // ToF_VL53L8CX::printProcessing();
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

        if (cmd.cmdStartsWith("MapBound"))
        {
          teleplot("MapBound", MapBoundaries[0]);
          teleplot("MapBound", MapBoundaries[1]);
          teleplot("MapBound", MapBoundaries[2]);
          teleplot("MapBound", MapBoundaries[3]);
        }
        if (cmd.cmdStartsWith("Pos"))
        {
          println("Pos x=%d y=%d h=%d", (int)Motion::GetCurrentPose().x, (int)Motion::GetCurrentPose().y, (int)Motion::GetCurrentPose().h);
        }
        if (cmd.cmdStartsWith("Speed"))
        {
          // print("Speed ", cmd);
          if (cmd.size > 0)
          {
            Motion::SetMaxSpeed(cmd.data[0]);
            // preferences.putInt("Speed",cmd.data[0]);
            println("Speed %f", Motion::GetMaxSpeed());
          }
          println("Motor D speed %f", Motion::motor_D.maxSpeed());
          println("Motor G speed %f", Motion::motor_G.maxSpeed());
        }
        if (cmd.cmdStartsWith("Accel"))
        {
          // print("Accel ", cmd);
          if (cmd.size > 0)
          {
            Motion::SetAcceleration(cmd.data[0]);
            // preferences.putInt("Accel",cmd.data[0]);
            println("Accel %f", Motion::GetAcceleration());
          }
          println("Motor D accel %f", Motion::motor_D.getAcceleration());
          println("Motor G accel %f", Motion::motor_G.getAcceleration());
        }
        if (cmd.cmdStartsWith("Go"))
        {
          // print("Go ", cmd);
          if (cmd.size == 1)
            Motion::Go(cmd.data[0]);
          if (cmd.size == 2)
            Motion::GoTo(cmd.data[0], cmd.data[1]);
          if (cmd.size == 3)
            Motion::GoTo(cmd.data[0], cmd.data[1], cmd.data[2]);
        }
        if (cmd.cmdStartsWith("Turn"))
        {
          // print("Turn ", cmd);
          if (cmd.size > 0)
            Motion::Turn(cmd.data[0]);
        }
        if (cmd.cmdStartsWith("Motor"))
        {
          println("Motor D");
          println("speed %f", Motion::motor_D.speed());
          println("acceleration %f", Motion::motor_D.getAcceleration());
          println("distanceToGo %i", (int)Motion::motor_D.distanceToGo());
          println("targetPosition %i", (int)Motion::motor_D.targetPosition());
          println("currentPosition %i", (int)Motion::motor_D.currentPosition());
          // println("computeNewSpeed",(long)Motion::motor_D.computeNewSpeed());
          println("-----");
          println("Motor G");
          println("speed %f", Motion::motor_G.speed());
          println("acceleration %f", Motion::motor_G.getAcceleration());
          println("distanceToGo %i", (int)Motion::motor_G.distanceToGo());
          println("targetPosition %i", (int)Motion::motor_G.targetPosition());
          println("currentPosition %i", (int)Motion::motor_G.currentPosition());
          // println("computeNewSpeed",(long)Motion::motor_G.computeNewSpeed());
          println("-----");
        }
      }
    }
    catch (std::exception const &e)
    {
      print("error ");
      println(e.what());
    }
    if (chrono.Check())
    {
      printChrono(chrono);
    }
    vTaskDelay(10);
  }
}
