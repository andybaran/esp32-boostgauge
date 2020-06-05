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

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);


BME280 mySensor;

// Bosch MAP sensor setup
int mapsen = 26; // Set MAP sensor input on Analog port 0 

// float value and value in bytes
typedef union {
  float psiPabsFloat;
  byte psiPabsByte[4];
} psiPabs;

psiPabs psiDifference;
//float psiPabs; // for converting our value in mbar to psi

//non blocking delay using millis from https://randomnerdtutorials.com/why-you-shouldnt-always-use-the-arduino-delay-function/
unsigned long previousMillis = 0;
const long interval = 250;


void setup() { 
  Serial1.begin(115200, SERIAL_8N1, 16, 17);
  Wire.begin();

  if (mySensor.beginI2C() == false) //Begin communication over I2C
  {
    Serial.println("BME280 did not respond. Please check wiring.");
    while(1); //Freeze
  }

  //mySensor.setFilter(4);
  //mySensor.setPressureOverSample(3);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  delay(2000);

} 
    
void loop() { 

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      float out_voltage = analogRead(mapsen) / 1000.000000; //esp32 is reading in millivolts so let's convert to volts for our curve to work
  
      // forumula derived from https://www.youtube.com/watch?v=83LuzJTIbAw&list=PLZ73CAxmn6f3F2Muguy2oU4sQEykYiTsw&index=5&t=375s
      float Pabs = 3294.11764705*(out_voltage/4.764)-279.99999999925; //4.764 measured as input voltage using multimeter
  
      // pressure in PSI from Bosch MAP minus BME280 + .5 as that seemed to be a consistent difference
      psiDifference.psiPabsFloat = ((Pabs / 68.947572932) - (mySensor.readFloatPressure() * 0.0001450377)) + .5;
      psiDifference.psiPabsFloat = floor(psiDifference.psiPabsFloat*10+0.5)/10; //round to nearest tenth

      if (psiDifference.psiPabsFloat != 0) {
        if (Serial1.availableForWrite() > 0) {
          Serial1.write(psiDifference.psiPabsByte,4);
        } else {
          Serial.println("The buffers are full");
        }
      }
        display.clearDisplay();
        display.setTextSize(2);
        display.setTextColor(WHITE);
        display.setCursor(0, 10);
        display.print(psiDifference.psiPabsFloat);
        display.print(" PSI");
        display.display(); 

  }
}
