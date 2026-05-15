#include "main.h"
#include <esp_task_wdt.h>

using namespace std;
using namespace Printer;
using namespace Hardware_Config;
using namespace ServoAX12;

Adafruit_INA219 ina219;

constexpr float fieldWidthMm = 3000.0f;

inline float MirrorX(float x)
{
  return fieldWidthMm - x;
}

// x, y, waitMs
struct PamiMovement
{
  int x;
  int y;
  int waitMs;
};

struct PamiConfig
{
  bool enabled;
  bool isNinja;
  int startX;
  int startY;
  int startHeading;
  PamiMovement initialMove;
  PamiMovement matchMove1;
  PamiMovement matchMove2;
};

constexpr int pamiConfigCount = 7;
static const PamiConfig pamiConfigs[pamiConfigCount] = {
  { true,  false, 60, 1890, -90, { 60, 1650, 0 }, { 60, 950, 0 }, { 0,0,0 } },
  { true,  false, 180, 1890, -90, {180, 1650, 0 }, {180, 1200, 0 }, { 700, 950,0 } },
  { true,  false, 420, 1890, -90, { 420, 1650, 0 }, { 420, 1320, 0 }, { 1350, 930, 0 } },
  { true,  false, 540, 1890, -90, { 540, 1650, 0 }, { 540, 1450, 0 }, { 1100, 1450, 0 } },
  { true,  true, 750, 1900,  -90, { 750, 1810, 0 }, { 1500, 1800, 0 }, { 0, 0, 0 } },
  { false, false,  0,    0, -90, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } },
  { false, false,  0,    0, -90, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } }
};

const PamiConfig *GetPamiConfig(int numPami)
{
  if (numPami < 1 || numPami > pamiConfigCount)
  {
    return nullptr;
  }
  const PamiConfig &config = pamiConfigs[numPami - 1];
  return config.enabled ? &config : nullptr;
}

float TeamAwareX(float x, IHM::Team team)
{
  return (team == IHM::Team::Bleu) ? MirrorX(x) : x;
}

void ApplyStartPose(const PamiConfig &config, IHM::Team team)
{
  Motion::SetCurrentRot(config.startHeading);
  Motion::SetCurrentX(TeamAwareX(config.startX, team));
  Motion::SetCurrentY(config.startY);
}

void ExecutePamiMovement(const PamiMovement &move, IHM::Team team)
{
  if (move.x == 0 || move.y == 0)
  {
    // No movement
    return;
  }
  const float targetX = TeamAwareX(move.x, team);
  Motion::GoTo(targetX, move.y);
  if (move.waitMs > 0)
  {
    delay(move.waitMs);
  }
}

void setup()
{
  ESP32_Helper::Initialisation();
  println("PAMI Firmware");

  Hardware::Initialisation(true);
  Power::EnablePower();

  Motion::Initialisation();

  // Valeurs par défaut : { ax12Id, {positions[0]=min ... positions[n-1]=max}, count }
  // Modifiables via commande : AX12Config:<nom>:<field>:<valeur>  (field: id|cnt|p0..p9)
  // Stockées en NVS, persistantes au redémarrage
  AddServo(ServoID::VL53, "VL53", ServoConfig(1, std::array<int32_t, MAX_SERVO_POSITIONS>{150, 150, 180, 150, 200}, 5));
  AddServo(ServoID::Bras, "Bras", ServoConfig(2, std::array<int32_t, MAX_SERVO_POSITIONS>{160, 160, 210, 160, 220}, 5));

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

int numPami = -1;
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
        numPami = Match::GetNumPami();
        Motion::SetOpponentChecking(false);
      }

      // En attente de retrait de la tirette pour démarrer le match
      if (Match::matchState == Match::State::MATCH_WAIT)
      {
        Motion::SetOpponentChecking(false);
        Screen::SetPose(Motion::GetCurrentPose());
        // Start Position
        // Save Y position and orientation

        // Motion::SetCurrentY(0);
        // Motion::SetCurrentX(Motion::centerPositionMm);
        Motion::SetCurrentRot(-90);

        const PamiConfig *config = GetPamiConfig(numPami);
        if (config)
        {
          ApplyStartPose(*config, IHM::team);
        }
        else
        {
          println("Invalid PAMI number %d in MATCH_WAIT", numPami);
        }
        //   else if (numPami == 2)
        //     setCurrentY(1710);
        //   else if (numPami == 3)
        //     setCurrentY(1603);
        //   else
        //     println("ERROR robot number");
      }

      // Match en cours
      if (Match::matchState == Match::State::MATCH_RUN)
      {
        println("-------");
        println("Start init position after 10 Sec !");

        ServoAX12::SetServoPosition(ServoID::VL53, ServoPosition::Pos2);
        ServoAX12::SetServoPosition(ServoID::Bras, ServoPosition::Pos1);

        if (IHM::switchMode == 1)
        {
          delay(10000);
        }
        else
        {
          delay(3000);
        }

        const PamiConfig *config = GetPamiConfig(numPami);
        if (config)
        {
          ExecutePamiMovement(config->initialMove, IHM::team);
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
            println("Match Time : %i", (int)(Match::getMatchTimeSec()));
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

        if (IHM::switchMode == 1)
        {
          // Motion::SetOpponentChecking(true);
          //  !!! Attention enchaine tous les mouvements d'un coup en cas de detection !!!
          Motion::SetOpponentChecking(true);
        }
        else
        {
          Motion::SetOpponentChecking(true);
        }

        if (config)
        {
          ExecutePamiMovement(config->matchMove1, IHM::team);
          ExecutePamiMovement(config->matchMove2, IHM::team);
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
            const float targetX = TeamAwareX(1630, IHM::team);
            Motion::GoTo(targetX, 1680);
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
        // Printer::teleplot("speed_D", Motion::motor_D.speed());
        // Printer::teleplot("speed_G", Motion::motor_G.speed());

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
            // preferences.putInt("Speed",cmd.data[0]);
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
            // preferences.putInt("Accel",cmd.data[0]);
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
        if (cmd.cmdStartsWith("RGB"))
        {
          // RGB:0:255:0
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
          // HSV:0:255:255
          //   if (cmd.size == 1)
          //   {
          //     led[0].setHue(cmd.data[0]);
          //   }
          //   if (cmd.size == 3)
          //   {
          //     led[0].setHSV(cmd.data[0], cmd.data[1], cmd.data[2]);
          //   }
          //   FastLED.show();
          //   print("HSV : ", led[0].red);
          //   print(" ", led[0].green);
          //   println(" ", led[0].blue);
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
