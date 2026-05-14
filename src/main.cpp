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

struct PamiMovement
{
  float x;
  float y;
  float heading;
  int waitMs;
};

struct PamiConfig
{
  bool enabled;
  float startX;
  float startY;
  float startHeading;
  PamiMovement initialMove;
  PamiMovement matchMove;
};

constexpr int pamiConfigCount = 7;
static const PamiConfig pamiConfigs[pamiConfigCount] = {
  { true,  60.0f, 1890.0f, -90.0f, { 60.0f, 1650.0f, -90.0f, 0 }, { 60.0f, 950.0f, -90.0f, 0 } },
  { true, 180.0f, 1890.0f, -90.0f, {180.0f, 1650.0f, -90.0f, 0 }, {700.0f, 950.0f, -90.0f, 0 } },
  { false,  0.0f,    0.0f, -90.0f, { 0.0f, 0.0f, -90.0f, 0 }, { 0.0f, 0.0f, -90.0f, 0 } },
  { false,  0.0f,    0.0f, -90.0f, { 0.0f, 0.0f, -90.0f, 0 }, { 0.0f, 0.0f, -90.0f, 0 } },
  { false,  0.0f,    0.0f, -90.0f, { 0.0f, 0.0f, -90.0f, 0 }, { 0.0f, 0.0f, -90.0f, 0 } },
  { false,  0.0f,    0.0f, -90.0f, { 0.0f, 0.0f, -90.0f, 0 }, { 0.0f, 0.0f, -90.0f, 0 } },
  { false,  0.0f,    0.0f, -90.0f, { 0.0f, 0.0f, -90.0f, 0 }, { 0.0f, 0.0f, -90.0f, 0 } }
};

const PamiConfig* GetPamiConfig(int numPami)
{
  if (numPami < 1 || numPami > pamiConfigCount)
  {
    return nullptr;
  }
  const PamiConfig& config = pamiConfigs[numPami - 1];
  return config.enabled ? &config : nullptr;
}

float TeamAwareX(float x, IHM::Team team)
{
  return (team == IHM::Team::Bleu) ? MirrorX(x) : x;
}

void ApplyStartPose(const PamiConfig& config, IHM::Team team)
{
  Motion::SetCurrentRot(config.startHeading);
  Motion::SetCurrentX(TeamAwareX(config.startX, team));
  Motion::SetCurrentY(config.startY);
}

void ExecutePamiMovement(const PamiMovement& move, IHM::Team team)
{
  const float targetX = TeamAwareX(move.x, team);
  Motion::GoTo(targetX, move.y, move.heading);
  if (move.waitMs > 0)
  {
    delay(move.waitMs);
  }
}

void ExecutePamiRun(const PamiConfig& config, IHM::Team team)
{
  ExecutePamiMovement(config.initialMove, team);
  ExecutePamiMovement(config.matchMove, team);
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

        const PamiConfig* config = GetPamiConfig(numPami);
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

        if(IHM::switchMode == 1)
        {
          delay(10000);
        }
        else
        {
          delay(3000);
        }

        const PamiConfig* config = GetPamiConfig(numPami);
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
        }

        println("-------");
        println("Start !");

        // if (IHM::switchMode == 1)
        //{
        println("Mode Match !");

        long speed = 0;
        long accel = 0;
        speed = Motion::maxSpeed;
        accel = Motion::maxAcceleration;

        // Motion::Turn(180);
        Motion::SetMaxSpeed(speed);
        Motion::SetAcceleration(accel);

        if(IHM::switchMode == 1)
        {
          //Motion::SetOpponentChecking(true);
          // !!! sinon enchaine tous les mouvements d'un coup en cas de detection !!!
          Motion::SetOpponentChecking(false);
        }
        else
        {
          Motion::SetOpponentChecking(false);
        }

        const PamiConfig* config2 = GetPamiConfig(numPami);
        if (config2)
        {
          ExecutePamiMovement(config2->matchMove, IHM::team);
        }
        else
        {
          println("Invalid PAMI number %d in MATCH_RUN second move", numPami);
        }

        ServoAX12::SetServoPosition(ServoID::Bras, ServoPosition::Pos2);
        //}

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
        static bool pos = false;
        if (!pos)
        {
          ServoAX12::SetServoPosition(ServoID::VL53,ServoPosition::Pos2, 5000);
          while(ServoAX12::IsServoMoving(ServoID::VL53))
          {
            vTaskDelay(100);
          }
          pos = true;
        }
        else
        {
          ServoAX12::SetServoPosition(ServoID::VL53,ServoPosition::Pos1, 5000);
          while(ServoAX12::IsServoMoving(ServoID::VL53))
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
