/*
 *  Mendel Sensors CO2 Manufacturing Reference Board
 */

#include <Wire.h>
#include <SPI.h>
//#include <SD.h>
#include "mendel_sensor.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Adafruit_Sensor.h>
#include "SparkFun_SCD30_Arduino_Library.h"


// Display Parameters
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_ADDR 0x3C // OLED display TWI address

// Create SCD
SCD30 SCD30;
mendel_sensor readings_SCD30;

// reset pin not used on 4-pin OLED module
Adafruit_SSD1306 display(-1);  // -1 = no reset pin

void setup() {
  Serial.begin(115200);
  while(!Serial);  // wait for serial terminal to open
  Serial.println();
  Serial.println(F("=== Mendel Sensor CO2 Manufacturing Reference Board ==="));
  
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) { // Address 0x3C for Geekcreit 128x64
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5,0);
  display.print("- Mendel -");
  display.display();

  // Init SCD30
  if (!SCD30.begin()) {
    Serial.println("-- SCD30 not found --");
    while (1);
  }
  Serial.println("-- Found SCD30 --");
 
}

void loop() {
  // SCD30
  if (SCD30.dataAvailable()) {
    readings_SCD30.setTempC(SCD30.getTemperature());
    readings_SCD30.setHumidity(SCD30.getHumidity());
    readings_SCD30.setCO2(SCD30.getCO2());
  }
  else {
    readings_SCD30.setTempC(-1.00);
    readings_SCD30.setHumidity(-1.00);
    readings_SCD30.setCO2(-1.00);
  }
  Serial.print("SCD30:   ");
  Serial.print(readings_SCD30.getTempF());
  Serial.print(", Humidity: ");
  Serial.print(readings_SCD30.getHumidity());
  Serial.print(", CO2: ");
  Serial.println(readings_SCD30.getCO2());

  displayParams(readings_SCD30.getTempF(), readings_SCD30.getHumidity(), readings_SCD30.getCO2());

  delay(3000);
}

void displayParams(float T, float H, float CO2) {

  // Updating scrolling text garbles it.  Stop it.
  display.stopscroll();
  delay(100);
  display.clearDisplay();
  display.display();
  delay(2000);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Line 1 - CO2
  display.setCursor(0, 0);
  display.print("CO2: ");
  display.print(CO2, 0); 

  // Line 2 - Temp
  display.setCursor (0, 12);
  display.print(T);
  display.print("F");

  // Line 3 - Humidity
  display.setCursor (0, 24);
  display.print(H);
  display.print("%");
  
  // Display everyting
  display.display(); 
}
