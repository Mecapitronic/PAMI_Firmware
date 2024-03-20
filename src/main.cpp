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
    Serial.begin(115200);
    delay(1000);
    Serial.println("PAMI Firmware");


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
            if(Serial.available()>0)
            {
                uint32_t tmpInt = Serial.read();
                xQueueSend(myQueue, &tmpInt, 0);
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
