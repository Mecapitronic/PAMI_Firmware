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
    
    // Sets the Sensor pins as Inputs
    pinMode(DETECT_1, INPUT);
    pinMode(DETECT_2, INPUT);


    // Sets the two stepper pins as Outputs
    pinMode(ENABLE, OUTPUT);
    pinMode(MS1, OUTPUT);
    pinMode(MS2, OUTPUT);
    pinMode(STEP_D, OUTPUT);
    pinMode(DIR_D, OUTPUT);
    pinMode(STEP_G, OUTPUT);
    pinMode(DIR_G, OUTPUT);

    digitalWrite(ENABLE,HIGH);

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

            digitalWrite(DIR_D, HIGH); // Enables the motor to move in a particular direction
            // Makes 200 pulses for making one full cycle rotation
            for (long x = 0; x < 1600; x++)
            {
                digitalWrite(STEP_D, HIGH);
                delayMicroseconds(700); // by changing this time delay between the steps we can change the rotation speed
                digitalWrite(STEP_D, LOW);
                delayMicroseconds(700);
            }
            delay(500); // One second delay

            digitalWrite(DIR_D, LOW); // Changes the rotations direction
            // Makes 400 pulses for making two full cycle rotation
            for (long x = 0; x < 3200; x++)
            {
                digitalWrite(STEP_D, HIGH);
                delayMicroseconds(500);
                digitalWrite(STEP_D, LOW);
                delayMicroseconds(500);
            }
            delay(500);
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
