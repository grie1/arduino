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

// TCA9548A Address
#define TCAADDR 0x70

// Display Parameters
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_ADDR 0x3C // OLED display TWI address

// Create SCD's.  Number should correspond to TCA port.
SCD30 SCD30;
mendel_sensor readings_SCD30_0;
mendel_sensor readings_SCD30_1;
mendel_sensor readings_SCD30_2;

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

  // Init SCD30 on TCA I2C Port 0
  tcaselect(0);
  if (!SCD30.begin()) {
    Serial.println("-- SCD30 not found on Port 0 --");
    while (1);
  }
  Serial.println("-- Found SCD30 on Port 0 --");

  // Init SCD30 on TCA I2C Port 1
  tcaselect(1);
  if (!SCD30.begin()) {
    Serial.println("-- SCD30 not found on Port 1 --");
    while (1);
  }
  Serial.println("-- Found SCD30 on Port 1 --");

  // Init SCD30 on TCA I2C Port 2
  tcaselect(2);
  if (!SCD30.begin()) {
    Serial.println("-- SCD30 not found on Port 2 --");
    while (1);
  }
  Serial.println("-- Found SCD30 on Port 2 --");
 
}

void loop() {
  // SCD30_0
  tcaselect(0);
  if (SCD30.dataAvailable()) {
    readings_SCD30_0.setTempC(SCD30.getTemperature());
    readings_SCD30_0.setHumidity(SCD30.getHumidity());
    readings_SCD30_0.setCO2(SCD30.getCO2());
  }
  else {
    readings_SCD30_0.setTempC(-1.00);
    readings_SCD30_0.setHumidity(-1.00);
    readings_SCD30_0.setCO2(-1.00);
  }
  Serial.print("SCD30_0:   ");
  Serial.print(readings_SCD30_0.getTempF());
  Serial.print(", Humidity: ");
  Serial.print(readings_SCD30_0.getHumidity());
  Serial.print(", CO2: ");
  Serial.println(readings_SCD30_0.getCO2());

    // SCD30_1
  tcaselect(1);
  if (SCD30.dataAvailable()) {
    readings_SCD30_1.setTempC(SCD30.getTemperature());
    readings_SCD30_1.setHumidity(SCD30.getHumidity());
    readings_SCD30_1.setCO2(SCD30.getCO2());
  }
  else {
    readings_SCD30_1.setTempC(-1.00);
    readings_SCD30_1.setHumidity(-1.00);
    readings_SCD30_1.setCO2(-1.00);
  }
  Serial.print("SCD30_1:   ");
  Serial.print(readings_SCD30_1.getTempF());
  Serial.print(", Humidity: ");
  Serial.print(readings_SCD30_1.getHumidity());
  Serial.print(", CO2: ");
  Serial.println(readings_SCD30_1.getCO2());

  // SCD30_2
  tcaselect(2);
  if (SCD30.dataAvailable()) {
    readings_SCD30_2.setTempC(SCD30.getTemperature());
    readings_SCD30_2.setHumidity(SCD30.getHumidity());
    readings_SCD30_2.setCO2(SCD30.getCO2());
  }
  else {
    readings_SCD30_2.setTempC(-1.00);
    readings_SCD30_2.setHumidity(-1.00);
    readings_SCD30_2.setCO2(-1.00);
  }
  Serial.print("SCD30_2:   ");
  Serial.print(readings_SCD30_2.getTempF());
  Serial.print(", Humidity: ");
  Serial.print(readings_SCD30_2.getHumidity());
  Serial.print(", CO2: ");
  Serial.println(readings_SCD30_2.getCO2());

  // Display Info
  float avgtemp = (readings_SCD30_0.getTempF() + readings_SCD30_1.getTempF() + readings_SCD30_2.getTempF()) / 3;
  float avghumidity = (readings_SCD30_0.getHumidity() + readings_SCD30_1.getHumidity() + readings_SCD30_2.getHumidity()) / 3;
  displayParams(avgtemp, avghumidity, readings_SCD30_0.getCO2(), readings_SCD30_1.getCO2(), readings_SCD30_2.getCO2());
  delay(3000);
}



void displayParams(float T, float H, float CO2_0, float CO2_1, float CO2_2) {

  // Updating scrolling text garbles it.  Stop it.
  display.stopscroll();
  delay(100);
  display.clearDisplay();
  display.display();
  delay(2000);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  
  // Line 1
  display.setCursor(0, 0);
  display.print("CO2_0: ");
  display.print(CO2_0, 0); 

  display.setCursor(8, 0);
  display.print("CO2_1: ");
  display.print(CO2_1, 0);

  // Line 2
  display.setCursor (0, 12);
  display.print(T);
  display.print("F");

  display.setCursor(8, 12);
  display.print("CO2_2: ");
  display.print(CO2_2, 0);  

  // Line 3
  display.setCursor (0, 24);
  display.print(H);
  display.print("%");
  
  // Display everyting
  display.display(); 
}

void tcaselect(uint8_t i) {
  if (i > 7) return;
 
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}
