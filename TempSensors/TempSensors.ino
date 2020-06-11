#include <Wire.h>
#include <SPI.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Adafruit_HTS221.h>

#include "Adafruit_MCP9808.h"
#include "Adafruit_ADT7410.h"
#include "Adafruit_SHT31.h"
#include "Adafruit_Si7021.h"
#include "Adafruit_HTU21DF.h"
#include "SparkFun_SCD30_Arduino_Library.h"

#define SEALEVELPRESSURE_HPA (1013.25)

unsigned long delayTime;

// Create Sensor Objects
Adafruit_BME280 bme; // I2C
Adafruit_MCP9808 MCP9808 = Adafruit_MCP9808();
Adafruit_ADT7410 ADT7410 = Adafruit_ADT7410();
Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_Si7021 Si7021 = Adafruit_Si7021(); // I2C 0x40, conflict with HTU21DF
Adafruit_HTS221 hts;
// Adafruit_HTU21DF htu = Adafruit_HTU21DF(); // I2C 0x40, conflict with Si7021
SCD30 SCD30;

void setup() {
  Serial.begin(115200);
  while(!Serial);  // wait for serial terminal to open
  Serial.println(F("Mendel Sensor Test"));

  if (!bme.begin()) {
    Serial.println("-- BME280 not found --");
    while (1);
  }
  Serial.println("-- Found BME280 --");
  setupBME280();

  if (!MCP9808.begin(0x18)) {
    Serial.println("-- MCP9808 not found --");
    while (1);
  }
  Serial.println("-- Found MCP9808 --");
  MCP9808.setResolution(3); // sets the resolution mode of reading, the modes are defined in the table bellow:
  // Mode Resolution SampleTime
  //  0    0.5°C       30 ms
  //  1    0.25°C      65 ms
  //  2    0.125°C     130 ms
  //  3    0.0625°C    250 ms

  if (!ADT7410.begin()) {
    Serial.println("Couldn't find ADT7410!");
    while (1);
  }
  delay(250); // delay for ADT7410 to init
  Serial.println("-- Found ADT7410 --");

  if (!sht31.begin(0x44)) {
    Serial.println("Couldn't find sht31!");
    while (1);
  }
  Serial.println("-- Found sht31 --");

  if (!Si7021.begin()) {
    Serial.println("Couldn't find Si7021!");
    while (1);
  }
  Serial.println("-- Found Si7021 --");
  setupSi7021();

  if (!hts.begin_I2C()) {
    Serial.println("-- HTS221 not found --");
    while (1);
  }
  Serial.println("-- Found HTS221 --");
  setupHTS221();

/*    if (!htu.begin()) {
    Serial.println("-- HTU21DF not found --");
    while (1);
  }
  Serial.println("-- Found HTU21DF --");
*/
 if (!SCD30.begin()) {
   Serial.println("-- SCD30 not found --");
   while(1);
 }
 Serial.println("-- Found SCD30 --");
 setupSCD30();

  Serial.println();
  delayTime=30000;
}

void loop() {

  // Temperature Measurements
  bme.takeForcedMeasurement(); // required for forced mode
  readBME280();
  readMCP9808();
  readADT7410();
  readSHT31();
  readSi7021();
  readHTS221();
//  readHTU21DF();
  readSCD30();
  Serial.println();
  
  delay(delayTime);
}

void readBME280() {
    Serial.print("BME280  Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.print(" *C, ");
    Serial.print((bme.readTemperature()*(1.8F))+32);
    Serial.print(" *F");
    Serial.print("; Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.print("; Pressure = ");
    Serial.print(bme.readPressure() / 100.0F);
    Serial.println();
  }


void setupBME280() {
    // humidity sensing
    Serial.println("    -- BME Set to Humidity Sensing Scenario --");
    Serial.println("    -- forced mode, 1x temperature / 1x humidity / 0x pressure oversampling --");
    Serial.println("    -- pressure off, filter off --");
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1,   // temperature
                    Adafruit_BME280::SAMPLING_X1, // pressure
                    Adafruit_BME280::SAMPLING_X1,   // humidity
                    Adafruit_BME280::FILTER_OFF );

    // suggested rate is 1Hz (1s)
  }

void readMCP9808() {
  float MCP9808_C = MCP9808.readTempC();
  float MCP9808_F = MCP9808.readTempF();
  Serial.print("MCP9808 Temperature = ");
  Serial.print(MCP9808_C, 2);
  Serial.print(" *C, ");
  Serial.print(MCP9808_F, 2);
  Serial.print(" *F");
  Serial.println();
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

void readSHT31() {
  float SHT31_C = sht31.readTemperature();
  float SHT31_F = SHT31_C * 9.0 / 5.0 +32;
  Serial.print("SHT31   Temperature = ");
  Serial.print(SHT31_C, 2);
  Serial.print(" *C, ");
  Serial.print(SHT31_F, 2);
  Serial.print(" *F");
  Serial.print("; Humidity = ");
  Serial.print(sht31.readHumidity());
  Serial.println();
}

void setupSi7021() {
  Serial.print("    -- Found model ");
  switch(Si7021.getModel()) {
    case SI_Engineering_Samples:
      Serial.print("SI engineering samples"); break;
    case SI_7013:
      Serial.print("Si7013"); break;
    case SI_7020:
      Serial.print("Si7020"); break;
    case SI_7021:
      Serial.print("Si7021"); break;
    case SI_UNKNOWN:
    default:
      Serial.print("Unknown");
  }
  Serial.print(" Rev(");
  Serial.print(Si7021.getRevision());
  Serial.print(")");
  Serial.print(" Serial #"); Serial.print(Si7021.sernum_a, HEX); Serial.println(Si7021.sernum_b, HEX);
}

void readSi7021() {
  float Si7021_C = Si7021.readTemperature();
  float Si7021_F = Si7021_C * 9.0 / 5.0 +32;
  Serial.print("Si7021  Temperature = ");
  Serial.print(Si7021_C, 2);
  Serial.print(" *C, ");
  Serial.print(Si7021_F, 2);
  Serial.print(" *F");
  Serial.print("; Humidity = ");
  Serial.print(Si7021.readHumidity());
  Serial.println();
}

void setupHTS221() {
  hts.setDataRate(HTS221_RATE_1_HZ);
  Serial.print("    -- HTS221 Data rate set to: ");
  switch (hts.getDataRate()) {
   case HTS221_RATE_ONE_SHOT: Serial.println(" One Shot"); break;
   case HTS221_RATE_1_HZ: Serial.println(" 1 Hz"); break;
   case HTS221_RATE_7_HZ: Serial.println(" 7 Hz"); break;
   case HTS221_RATE_12_5_HZ: Serial.println(" 12.5 Hz"); break;
  }
}

void readHTS221() {
  sensors_event_t HTS221_temp;
  sensors_event_t HTS221_humidity;
  hts.getEvent(&HTS221_humidity, &HTS221_temp);
  float HTS221_C = HTS221_temp.temperature;
  float HTS221_F = HTS221_C * 9.0 / 5.0 + 32;
  Serial.print("HTS221  Temperature = ");
  Serial.print(HTS221_C);
  Serial.print(" *C, ");
  Serial.print(HTS221_F);
  Serial.print(" *F");
  Serial.print("; Humidity = ");
  Serial.print(HTS221_humidity.relative_humidity); 
  Serial.println();
}

/*
void readHTU21DF() {
  float HTU21DF_C = htu.readTemperature();
  float HTU21DF_F = HTU21DF_C * 9.0 / 5.0 +32;
  Serial.print("HTU21DF Temperature = ");
  Serial.print(HTU21DF_C, 2);
  Serial.print(" *C, ");
  Serial.print(HTU21DF_F, 2);
  Serial.print(" *F");
  Serial.println();
}
*/
void readSCD30() {
  if (SCD30.dataAvailable()) {
    Serial.print("SCD30   Temperature = ");
    Serial.print(SCD30.getTemperature());
    Serial.print(" *C, ");
    Serial.print((SCD30.getTemperature()*(1.8F))+32);
    Serial.print(" *F");
    Serial.print("; Humidity = ");
    Serial.print(SCD30.getHumidity());
    Serial.print("; CO2 = ");
    Serial.print(SCD30.getCO2());
    Serial.println();
  }
  else {
    Serial.print("SCD30   No Data Available");
    Serial.println();
  }
}

void setupSCD30() {
  int SCD30MeasurementInterval = 20; // 2 to 1800 sec
  int SCD30AltitudeCompensation = 14; // altiude
  float SCD30TemperatureOffset = 1.75; // up to 5 *C
  // pressure set in define SEALEVELPRESSURE_HPA 700 to 1200
  
  SCD30.setMeasurementInterval(SCD30MeasurementInterval);
  Serial.print("    -- SCD30 Measurement Interval Set to: ");
  Serial.print(SCD30MeasurementInterval);
  Serial.println(" sec");  
  
  SCD30.setAltitudeCompensation(SCD30AltitudeCompensation); // Cerritos 14m above sea level
  Serial.print("    -- SCD30 Altitude Compensation Set to: ");
  Serial.print(SCD30AltitudeCompensation);
  Serial.println(" m");
  
  SCD30.setAmbientPressure(SEALEVELPRESSURE_HPA); // Sea level pressure (see define) between 700 to 1200
  Serial.print("    -- SCD30 Ambient Pressure Set to: ");
  Serial.println(SEALEVELPRESSURE_HPA);

  SCD30.setTemperatureOffset(SCD30TemperatureOffset); // Up to 5° C
  float SCD30_temp_offset = SCD30.getTemperatureOffset();
  Serial.print("    -- SCD30 Temperature offset: ");
  Serial.print(SCD30_temp_offset,2);
  Serial.println(" *C");  
}
