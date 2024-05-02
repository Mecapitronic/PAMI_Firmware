#include "main.h"


void setup()
{
  Serial.end();
  Serial.begin(921600);
  delay(1000);
  Serial.println("PAMI Firmware");
  
  initMotion();

  setMaxSpeed(4000);
  setAcceleration(MAX_ACCELERATION);  

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
  while(motor_D.isRunning() || motor_G.isRunning())
  {
    enableMotors();
    updateMotors();
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

        Serial.println("Go 1");
        go(3000);
        delay(1000);

        Serial.println("Go 2");
        go(-1000);
        delay(1000);
        
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

    }
    catch (std::exception const &e)
    {
      Serial.print("error : ");
      Serial.println(e.what());
    }
    vTaskDelay(1);
  }
}
