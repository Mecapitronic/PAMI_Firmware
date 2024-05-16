#include "sensors.h"

//VL53L0X sensors[2];
uint16_t sensor1 = 3000;
uint16_t sensor2 = 3000;

long previousTime = 0;

void initSensor()
{
    // Set ADC unit resolution on ESP32
    //analogReadResolution(12);
    //analogSetWidth(12);   
}

int readSensors()
{
    int distance = 0;


    // float volts = analogRead( 35 ) * (3.3 / 4096);

    // int distance_cm = 29.988 * pow( volts , -1.173)

    int value = analogRead(SHARP_1);
    /* Conversion Analogique en mm*/
    distance = COEF_A / (value - COEF_B);
    /* Écrêtage */
    if (distance > COEF_C || distance <= 1) {
        distance = COEF_C;
    }
    return distance;
}

bool checkOpponent(uint16_t distance)
{
    //readSensors();
    //if (sensor1 <= distance || sensor2 <= distance) return true;

    Enable tiretteTmp = (Enable)!digitalRead(PIN_TIRETTE);
    
    if (tiretteTmp == ENABLE_TRUE)
        return true;
    else 
        return false;
}