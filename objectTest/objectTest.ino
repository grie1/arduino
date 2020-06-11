
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Adafruit_ADT7410.h"
#include "SparkFun_SCD30_Arduino_Library.h"
#include "RTClib.h"
#include "mendel_sensor.h"

// Sensor Objects
SCD30 airSensor;

// Data Variable Objects
mendel_sensor sensor_SCD30;

void setup() {
Serial.begin(115200);
  while(!Serial);  // wait for serial terminal to open
  Serial.println();
  Serial.println(F("SCD30 Low Power Forced Recalibration"));

  sensor_SCD30.setCO2(4.544);
  Serial.print("CO2: ");
  Serial.println(sensor_SCD30.getCO2());

  sensor_SCD30.setTempC(28.000);
  Serial.println(sensor_SCD30.getTempC());
  Serial.println(sensor_SCD30.getTempF());
  Serial.println(sensor_SCD30.getCO2());
}

void loop() {
  // put your main code here, to run repeatedly:

}
