#include "sensors.h"

//VL53L0X sensors[2];
uint16_t sensor1 = 3000;
uint16_t sensor2 = 3000;
int pamiNum1=0;
int pamiNum2=0;
int pamiNum=0;
long previousTime = 0;

void initSensor()
{
    // Set ADC unit resolution on ESP32
    //analogReadResolution(12);
    //analogSetWidth(12);

  //Sets the Sensor pins as Inputs
  pinMode(PIN_SENSOR_1, INPUT_PULLUP);


  //Selecteur de PAMI
  pinMode(PIN_PAMI_NUM_1, INPUT_PULLUP);
  pinMode(PIN_PAMI_NUM_2, INPUT_PULLUP);
}

int readSensors()
{
    int distance = 0;


    // float volts = analogRead( 35 ) * (3.3 / 4096);

    // int distance_cm = 29.988 * pow( volts , -1.173)

    //int value = analogRead(SHARP_1);
    int value = 0;
    /* Conversion Analogique en mm*/
    distance = COEF_A / (value - COEF_B);
    /* Écrêtage */
    if (distance > COEF_C || distance <= 1) {
        distance = COEF_C;
    }
    return distance;
}

bool checkSensor()
{
    //readSensors();
    //if (sensor1 <= distance || sensor2 <= distance) return true;

    bool obs = digitalRead(PIN_SENSOR_1);
    
    return obs;
}

int ReadNumPami()
{
    pamiNum1 = digitalRead(PIN_PAMI_NUM_1);
    pamiNum2 = digitalRead(PIN_PAMI_NUM_2);
    pamiNum = (pamiNum2 << 1) | pamiNum1;
    return pamiNum;
}

int GetNumPami()
{
    return pamiNum;
}