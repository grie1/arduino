/*

SCD30 Forced Recalibration in Low Power Mode
1. Set sampling interval.  
2. Let device run 5x sampling interval (pg. 2 top right low power mode pdf)
3. Force recalibration
4. Set temperature offset

*/

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Adafruit_ADT7410.h"
#include "SparkFun_SCD30_Arduino_Library.h"
#include "RTClib.h"
#include "mendel_sensor.h"

// Display Parameters
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_ADDR 0x3C // OLED display TWI address

// Globals
float SCD30_CO2;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char dateBuffer[12];
long interval_delay;

// Create objects
SCD30 airSensor;
Adafruit_ADT7410 ADT7410 = Adafruit_ADT7410();
RTC_DS3231 rtc;

// Data Variable Objects
mendel_sensor SCD30_readings;

// reset pin not used on 4-pin OLED module
Adafruit_SSD1306 display(-1);  // -1 = no reset pin

void setup() {
  Serial.begin(115200);
  while(!Serial);  // wait for serial terminal to open
  Serial.println();
  Serial.println(F("SCD30 Low Power Forced Recalibration"));

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) { // Address 0x3C for Geekcreit 128x64
    Serial.println(F("SSD1306 allocation failed"));
    while (1);
  }
  display.clearDisplay();
  display.display();

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    abort();
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting to date & time sketch was compiled");
    // Sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  // Wire.begin();

  if (!airSensor.begin()) {
    Serial.println("-- SCD30 not found --");
    while (1);
  }
  Serial.println("-- Found SCD30 --");
  SCD30_LowPowerModeSetup();


  if (!ADT7410.begin()) {
   Serial.println("Couldn't find ADT7410!");
   while (1);
  }
  delay(250); // delay for ADT7410 to init
  Serial.println("-- Found ADT7410 --");
 
}

void loop() {
  readSCD30();
  readADT7410(); 
  DateTime now = rtc.now();
  sprintf(dateBuffer, "%04u-%02u-%02u ",now.year(),now.month(),now.day());
  Serial.print(dateBuffer);
  sprintf(dateBuffer,"%02u:%02u:%02u ",now.hour(),now.minute(),now.second());
  Serial.println(dateBuffer);

  float SCD30_nvram_offset = airSensor.getTemperatureOffset();
  Serial.print("    -- SCD30 Temperature offset: ");
  Serial.print(SCD30_nvram_offset,2);
  Serial.println(" *C"); 
  Serial.println();
  
  displayCO2();   
  
  delay(25000);
}

void readSCD30() {
  if (airSensor.dataAvailable()) {
    SCD30_CO2 = airSensor.getCO2();
    Serial.print("SCD30   Temperature = ");
    Serial.print(airSensor.getTemperature());
    Serial.print(" *C, ");
    Serial.print((airSensor.getTemperature()*(1.8F))+32);
    Serial.print(" *F");
    Serial.print("; Humidity = ");
    Serial.print(airSensor.getHumidity());
    Serial.print("; CO2 = ");
    //Serial.print(SCD30_CO2);
    Serial.print(SCD30_CO2);
    Serial.println();
  }
  else {
    Serial.print("SCD30   No Data Available");
    Serial.println();
  }
}

void SCD30_LowPowerModeSetup() {
  int CO2_concentration = 400;
  int SCD30_measurement_interval = 20; //Change number of seconds between measurements: 2 to 1800 (30 minutes)
  int SCD30_altitude = 14; //Set altitude of the sensor in m
  int SCD30_pressure = 1009; //Current ambient pressure in mBar: 700 to 1200
  float SCD30_temp_offset = 1.75; // Temperature offset, up to 5°C

  interval_delay = SCD30_measurement_interval * 5 * 100;

  airSensor.setMeasurementInterval(SCD30_measurement_interval); 
  delay(interval_delay);
  Serial.print("    -- SCD30 Measurement Interval Set to: ");
  Serial.print(SCD30_measurement_interval);
  Serial.println(" sec");  
  Serial.print("    -- Sleeping 5x measurement interval: ");
  Serial.print(interval_delay / 100);
  Serial.println(" sec");  

/*
  Serial.print("    -- Forcing SCD30 calibration at: ");
  Serial.print(CO2_concentration);
  Serial.println(" ppm");
  airSensor.setForcedRecalibrationFactor(CO2_concentration);
  delay(2000);
*/

  airSensor.setAltitudeCompensation(SCD30_altitude); 
  delay(1500);
  Serial.print("    -- SCD30 Altitude Compensation Set to: ");
  Serial.print(SCD30_altitude);
  Serial.println(" m");
  
  airSensor.setAmbientPressure(SCD30_pressure); 
  delay(1500);
  airSensor.setAmbientPressure(SCD30_pressure); // Sea level pressure (see define) between 700 to 1200
  Serial.print("    -- SCD30 Ambient Pressure Set to: ");
  Serial.println(SCD30_pressure);

  airSensor.setTemperatureOffset(SCD30_temp_offset); 
  delay(1500);
  float SCD30_nvram_offset = airSensor.getTemperatureOffset();
  Serial.print("    -- SCD30 Temperature offset: ");
  Serial.print(SCD30_nvram_offset,2);
  Serial.println(" *C");  
}

void readADT7410() {
  float ADT7410_C = ADT7410.readTempC();
  float ADT7410_F = ADT7410_C * 9.0 / 5.0 +32;
  Serial.print("ADT7410 Temperature = ");
  Serial.print(ADT7410_C, 2);
  Serial.print(" *C, ");
  Serial.print(ADT7410_F, 2);
  Serial.print(" *F");
  Serial.println();
}
void displayCO2(void) {

  // Get for local function
  DateTime now = rtc.now();
  
  // Updating scrolling text garbles it.  Stop it.
  display.stopscroll();
  delay(100);
  display.clearDisplay();
  display.display();
  delay(2000);

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  
  // Line 1 - Date Time
  display.setCursor(0, 0);
  display.print(now.month(), DEC);
  display.print('/');
  display.print(now.day(), DEC);
  display.print(": ");
  if(now.hour() < 10) {
       display.print("0");
  }
  display.print(now.hour(), DEC);
  display.print(':');
  if (now.minute() < 10) {
    display.print("0");
  }
  display.print(now.minute(), DEC);
    
  // Line 2 - CO2
  display.setCursor(0, 16);
  display.print("CO2: ");
  display.print(SCD30_CO2, 0); 

  // Line 3 - Temp
  display.setCursor (0, 32);
  display.print(airSensor.getTemperature());
  display.print("C");

  // Line 4 - Humidity
  display.setCursor (0, 48);
  display.print(airSensor.getHumidity());
  display.print("%");
  
  // Display everyting
  display.display(); 

  // Scroll
  //display.startscrollright(0x00, 0x0F);
}

void displayTime(void) {
  
}
