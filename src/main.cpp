#include "main.h"

using namespace std;
using namespace Printer;
using namespace Hardware_Config;

Preferences preferences;
Adafruit_INA219 ina219;

void setup()
{
  ESP32_Helper::Initialisation();
  println("PAMI Firmware");

  // Normal speed is 100 000
  // With higher speed, instructions on I2C take less time
  Wire.begin(SDA, SCL, 400000UL);

  PowerMonitor::Initialisation();

  Screen::Initialisation();
  Screen::Logo();
  
  IHM::Initialisation();
  Match::Initialisation();

  initSensor();
  initMotion();

  ServoAX12::Initialisation(SERIAL_SERVO, RX_SERVO, TX_SERVO, PIN_SERVO_DIR);
  ServoAX12::AddServo(ServoID::VL53, "VL53", ServoPosition::VL53Min, ServoPosition::VL53Max);
  ServoAX12::SetServoPosition(ServoID::VL53, ServoPosition::VL53Pos);
  ServoAX12::AddServo(ServoID::Bras, "Bras", ServoPosition::BrasMin, ServoPosition::BrasMax);
  ServoAX12::SetServoPosition(ServoID::Bras, ServoPosition::BrasPos);
  
  ToF_VL53L8CX::Initialisation();

  // delay(2000);

  setMaxSpeed(MAX_SPEED);
  setAcceleration(MAX_ACCELERATION);

  //int speedPref = preferences.getInt("Speed",0);
  //int accelPref = preferences.getInt("Accel",0);

  // if(speedPref != 0)
  // {
  //   setMaxSpeed(speedPref);
  //   println("Speed : %i", speedPref);
  // }
  // if(accelPref != 0)
  // {
  //   setAcceleration(accelPref);
  //   println("Accel : %i", accelPref);
  // }

  /* Task function. */
  /* name of task. */
  /* Stack size of task */
  /* parameter of the task */
  /* priority of the task */
  /* Task handle to keep track of created task */
  /* pin task to core 0 */
  xTaskCreatePinnedToCore(TaskMatch, "TaskMatch", 20000, NULL, 10, &Task1, 0);
  xTaskCreatePinnedToCore(TaskSerial, "TaskSerial", 20000, NULL, 5, &Task2, 0);
}

void loop()
{
  if (Match::matchState != Match::State::MATCH_STOP && Match::matchState != Match::State::MATCH_END && (motor_D.isRunning() || motor_G.isRunning()))
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
void TaskMatch(void *pvParameters)
{
  println("Start TaskMatch");

  while (1)
  {
    try
    {
      // En attente de retrait de la tirette pour démarrer le match
      if (Match::matchState == Match::State::MATCH_WAIT)
      {
        // Lecture do codage du numéro de PAMI
        int numPamiTmp = ReadNumPami();
        if (numPamiTmp != numPami)
        {
          numPami = numPamiTmp;
          println("N° PAMI : %i", numPami);
          Wifi_Helper::SetLocalIP("192.168.137." + String(100 + numPami + 1));
        }

        // Start Position
        // Save Y position and orientation

        setCurrentY(0);
        setCurrentX(CENTER_POSITION_MM);
        setCurrentRot(0);

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
        //   setCurrentX(CENTER_POSITION_MM);
        //   setCurrentRot(0);
        // }
        // else
        // {
        //   setCurrentX(3000-CENTER_POSITION_MM);
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
          // setOpponentChecking(true);

          speed = micros() % (int)(MAX_SPEED * 3 / 4) + (int)(MAX_SPEED * 1 / 4);
          accel = micros() % (int)(MAX_ACCELERATION * 3 / 4) + (int)(MAX_ACCELERATION * 1 / 4);
          println("speed : %i", speed);
          println("accel : %i", accel);
          setMaxSpeed(speed);
          setAcceleration(accel);

          go(800);

          speed = micros() % (int)(MAX_SPEED * 3 / 4) + (int)(MAX_SPEED * 1 / 4);
          accel = micros() % (int)(MAX_ACCELERATION * 3 / 4) + (int)(MAX_ACCELERATION * 1 / 4);
          println("speed : %i", speed);
          println("accel : %i", accel);
          setMaxSpeed(speed);
          setAcceleration(accel);

          turn(180);

          speed = micros() % (int)(MAX_SPEED * 3 / 4) + (int)(MAX_SPEED * 1 / 4);
          accel = micros() % (int)(MAX_ACCELERATION * 3 / 4) + (int)(MAX_ACCELERATION * 1 / 4);
          println("speed : %i", speed);
          println("accel : %i", accel);
          setMaxSpeed(speed);
          setAcceleration(accel);

          go(1000);
        }
        else
        {
          println("Mode Test !");

          long speed = 0;
          long accel = 0;
          // setOpponentChecking(true);

          speed = MAX_SPEED;        // micros()%(int)(MAX_SPEED*3/4)+(int)(MAX_SPEED*1/4);
          accel = MAX_ACCELERATION; // micros()%(int)(MAX_ACCELERATION*3/4)+(int)(MAX_ACCELERATION*1/4);
          println("speed : %i", speed);
          println("accel : %i", accel);
          setMaxSpeed(speed);
          setAcceleration(accel);

          turn(45);
          turn(-90);
          turn(45);
          turn(-360);
        }

        println("Stop !");
        println("------");

        /*
        if (numPami == 0)
        {
          setOpponentChecking(true);
          if (team == Team::TEAM_YELLOW)
          {
            goTo(647, 1924);
            setMaxSpeed(MAX_SPEED/3);
            setAcceleration(MAX_ACCELERATION/3);
            goTo(1250, 1924);
            setMaxSpeed(MAX_SPEED/2);
            setAcceleration(MAX_ACCELERATION/2);
            turnTo(1250, 1580);
            go(-100);
            setCurrentY(2000-CENTER_POSITION_MM);
            setMaxSpeed(MAX_SPEED);
            setAcceleration(MAX_ACCELERATION);
            goTo(1250, 1580);
          }
          else
          {
            goTo(3000 - 647, 1924);
            setMaxSpeed(MAX_SPEED/3);
            setAcceleration(MAX_ACCELERATION/3);
            goTo(3000 - 1250, 1924);
            setMaxSpeed(MAX_SPEED/2);
            setAcceleration(MAX_ACCELERATION/2);
            turnTo(3000 - 1250, 1580);
            go(-100);
            setCurrentY(2000-CENTER_POSITION_MM);
            setMaxSpeed(MAX_SPEED);
            setAcceleration(MAX_ACCELERATION);
            goTo(3000 - 1250, 1580);
          }
        }
        else if (numPami == 1)
        {
          delay(3000);
          if (team == Team::TEAM_YELLOW)
          {
            goTo(350, 1817);
            turnTo(750, 1500);
            goTo(750, 1500);
          }
          else
          {
            goTo(3000 - 350, 1817);
            turnTo(3000 - 750, 1500);
            goTo(3000 - 750, 1500);
          }
        }
        else if (numPami == 2)
        {
          if (team == Team::TEAM_YELLOW)
          {
            goTo(375, 1710);
            turnTo(1500, 1250);
            setMaxSpeed(MAX_SPEED/2);
            setAcceleration(MAX_ACCELERATION/2);
            goTo(1500, 1250);
          }
          else
          {
            goTo(3000 - 375, 1710);
            turnTo(3000 - 1500, 1250);
            setMaxSpeed(MAX_SPEED/2);
            setAcceleration(MAX_ACCELERATION/2);
            goTo(3000 - 1500, 1250);
          }
        }
        else if (numPami == 3)
        {
          if (team == Team::TEAM_YELLOW)
          {
            goTo(550, 1603);
            turnTo(1500, 1250);
          }
          else
          {
            goTo(3000 - 550, 1603);
            turnTo(3000 - 1500, 1250);
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
        if(IHM::switchMode == 0 && IHM::tirettePresent == 0)
          Match::matchState = Match::State::MATCH_BOOT;
        
      }
    }
    catch (std::exception const &e)
    {
      printError(e.what());
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
  int lastMatchTime = 0;

  while (1)
  {
    try
    {
      if (teleplotTO.IsTimeOut() && Match::matchState != Match::State::MATCH_RUN)
      {
        // Printer::teleplot("pos", getCurrentPose());
        // Printer::teleplot("ang", (int)(getCurrentPose().h));

        // Countdown
        if (lastMatchTime != (int)(Match::getMatchTimeSec()))
        {
          println("Match Time : %i", (int)(Match::getMatchTimeSec()));
          lastMatchTime = (int)(Match::getMatchTimeSec());
        }
        //Printer::teleplot("mapBoundaries", MapBoundaries[0]);
        //Printer::teleplot("mapBoundaries", MapBoundaries[1]);
        //Printer::teleplot("mapBoundaries", MapBoundaries[2]);
        //Printer::teleplot("mapBoundaries", MapBoundaries[3]);
      }

      // Check if we get commands from operator via debug serial
      if (ESP32_Helper::HasWaitingCommand())
      {
        Command cmd = ESP32_Helper::GetCommand();

        if (cmd.cmd.startsWith("Pos"))
        {
          print("Pos : x=%f", getCurrentPose().x);
          print("  y=%f", getCurrentPose().y);
          print("  h=%f", getCurrentPose().h);
          println();
        }
        if (cmd.cmd.startsWith("Speed"))
        {
          // print("Speed : ", cmd);
          if (cmd.size > 0)
          {
            setMaxSpeed(cmd.data[0]);
            //preferences.putInt("Speed",cmd.data[0]);
            println("Speed : %f", getMaxSpeed());
          }
          println("Motor D speed: %f", motor_D.maxSpeed());
          println("Motor G speed: %f", motor_G.maxSpeed());
        }
        if (cmd.cmd.startsWith("Accel"))
        {
          // print("Accel : ", cmd);
          if (cmd.size > 0)
          {
            setAcceleration(cmd.data[0]);
            //preferences.putInt("Accel",cmd.data[0]);
            println("Accel : %f", getAcceleration());
          }
          println("Motor D accel: %f", motor_D.acceleration());
          println("Motor G accel: %f", motor_G.acceleration());
        }
        if (cmd.cmd.startsWith("Pulse"))
        {
          // print("Pulse : ", cmd);
          if (cmd.size > 0)
          {
            motor_D.setMinPulseWidth(cmd.data[0]);
            motor_G.setMinPulseWidth(cmd.data[0]);
          }
          println("setMinPulseWidth: %i", cmd.data[0]);
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
          println("speed: %f", motor_D.speed());
          println("acceleration: %f", motor_D.acceleration());
          println("distanceToGo: %i", (int)motor_D.distanceToGo());
          println("targetPosition: %i", (int)motor_D.targetPosition());
          println("currentPosition: %i", (int)motor_D.currentPosition());
          // println("computeNewSpeed:",(long)motor_D.computeNewSpeed());
          println("-----");
          println("Motor G:");
          println("speed: %f", motor_G.speed());
          println("acceleration: %f", motor_G.acceleration());
          println("distanceToGo: %i", (int)motor_G.distanceToGo());
          println("targetPosition: %i", (int)motor_G.targetPosition());
          println("currentPosition: %i", (int)motor_G.currentPosition());
          // println("computeNewSpeed:",(long)motor_G.computeNewSpeed());
          println("-----");
        }
        if (cmd.cmd.startsWith("Blink"))
        {
          //Blink:0
          //Blink:1
          if (cmd.size > 0)
          {
            IHM::useBlink = cmd.data[0];
            println("Blink : %i", IHM::useBlink);
          }
        }
        if (cmd.cmd.startsWith("RGB"))
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
        if (cmd.cmd.startsWith("HSV"))
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
    vTaskDelay(1);
    // vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
