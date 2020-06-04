/*
 * Pressure Sensor being used purchased from https://www.amazon.com/gp/product/B07L9S65B3/ref=ppx_yo_dt_b_asin_title_o06_s00?ie=UTF8&psc=1
 * Item model number  038906051C
 * Manufacturer Part Number  038906051C
 * OEM Part Number   0281002401, 038 906 051 C, 038906051C 
 * 
 * TODO : Figure out curve for temperature sensor and log air temp
 * 
 */

#include <Wire.h>
#include "SparkFunBME280.h"
BME280 mySensor;

// Bosch MAP sensor setup
int mapsen = 26; // Set MAP sensor input on Analog port 0 
float psiPabs; // for converting our value in mbar to psi

//non blocking delay using millis from https://randomnerdtutorials.com/why-you-shouldnt-always-use-the-arduino-delay-function/
unsigned long previousMillis = 0;
const long interval = 250;

void setup() { 
  Serial.begin(115200);
  Wire.begin();

  if (mySensor.beginI2C() == false) //Begin communication over I2C
  {
    Serial.println("BME280 did not respond. Please check wiring.");
    while(1); //Freeze
  }

  mySensor.setFilter(4);
  mySensor.setPressureOverSample(3);

} 
    
void loop() { 

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      float out_voltage = analogRead(mapsen) / 1000.000000; //esp32 is reading in millivolts so let's convert to volts for our curve to work
  
      // forumula derived from https://www.youtube.com/watch?v=83LuzJTIbAw&list=PLZ73CAxmn6f3F2Muguy2oU4sQEykYiTsw&index=5&t=375s
      float Pabs = 3294.11764705*(out_voltage/4.764)-279.99999999925; //4.764 measured as input voltage using multimeter
  
      // pressure in PSI from Bosch MAP minus BME280 + .5 as that seemed to be a consistent difference
      psiPabs = ((Pabs / 68.947572932) - (mySensor.readFloatPressure() * 0.0001450377)) + .5;//(mpr.readPressure() / 68.947572932); 

      Serial.println(psiPabs);

  }
}
