/*
 *  Mendel Sensors Datalogger
*/

#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "mendel_sensor.h"
#include "RTClib.h"
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
// #include <Adafruit_HTS221.h>
#include <Adafruit_MPL115A2.h>
#include "Adafruit_MCP9808.h"
#include "Adafruit_ADT7410.h"
#include "Adafruit_SHT31.h"
#include "Adafruit_Si7021.h"
//#include "Adafruit_HTU21DF.h"
#include "SparkFun_SCD30_Arduino_Library.h"


#define SEALEVELPRESSURE_HPA (1013.25)

// Display Parameters
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_ADDR 0x3C // OLED display TWI address

// RTC
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char dateBuffer[12];

// SDCard SS/CS Pin
const int chipSelect = 53;
// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;
File dataFile;

Adafruit_BME280 bme; // I2C
Adafruit_MCP9808 MCP9808 = Adafruit_MCP9808();
Adafruit_ADT7410 ADT7410 = Adafruit_ADT7410();
Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_Si7021 Si7021 = Adafruit_Si7021(); // I2C 0x40, conflict with HTU21DF
Adafruit_MPL115A2 mpl115a2;
// Adafruit_HTS221 hts;
// Adafruit_HTU21DF htu = Adafruit_HTU21DF(); // I2C 0x40, conflict with Si7021
SCD30 SCD30;
RTC_DS3231 rtc;

// reset pin not used on 4-pin OLED module
Adafruit_SSD1306 display(-1);  // -1 = no reset pin

// Create Measurement Objects
mendel_sensor readings_BME280;
mendel_sensor readings_MCP9808;
mendel_sensor readings_ADT7410;
mendel_sensor readings_sht31;
mendel_sensor readings_Si7021;
mendel_sensor readings_mpl115a2;
mendel_sensor readings_SCD30;

void setup() {
  Serial.begin(115200);
  while(!Serial);  // wait for serial terminal to open
  Serial.println();
  Serial.println(F("======== Mendel Sensor Data Logger ========"));

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
  
  Serial.print("Initializing SD card...");
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");
  
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

  mpl115a2.begin();
     
  if (!SCD30.begin()) {
    Serial.println("-- SCD30 not found --");
    while (1);
  }
  Serial.println("-- Found SCD30 --");
  setupSCD30();

  sdCardInit();

  Serial.println();
}

void loop() {

  DateTime now = rtc.now();
  sprintf(dateBuffer, "%04u-%02u-%02u ",now.year(),now.month(),now.day());
  Serial.print(dateBuffer);
  sprintf(dateBuffer,"%02u:%02u:%02u ",now.hour(),now.minute(),now.second());
  Serial.println(dateBuffer);

  //mendel_sensor::displayParameters(readings_BME280

  // BME280
  bme.takeForcedMeasurement();
  readings_BME280.setTempC(bme.readTemperature());
  readings_BME280.setPressure(bme.readPressure() / 100.0F);
  readings_BME280.setHumidity(bme.readHumidity());
  Serial.print("BME280:  ");
  Serial.print(readings_BME280.getTempF());
  Serial.print(", Humidity: ");
  Serial.print(readings_BME280.getHumidity());  
  Serial.print(", Pressure: ");
  Serial.println(readings_BME280.getPressure());

  // MCP9808
  readings_MCP9808.setTempC(MCP9808.readTempC());
  Serial.print("MCP9808: ");
  Serial.println(readings_MCP9808.getTempF());

  // ADT7410
  readings_ADT7410.setTempC(ADT7410.readTempC());
  Serial.print("ADT7410: ");
  Serial.println(readings_ADT7410.getTempF());

  // SHT31-D
  readings_sht31.setTempC(sht31.readTemperature());
  readings_sht31.setHumidity(sht31.readHumidity());
  Serial.print("SHT31:   ");
  Serial.print(readings_sht31.getTempF());
  Serial.print(", Humidity: ");
  Serial.println(readings_sht31.getHumidity());

  // Si7021
  readings_Si7021.setTempC(Si7021.readTemperature());
  readings_Si7021.setHumidity(Si7021.readHumidity());
  Serial.print("Si7021:  ");
  Serial.println(readings_Si7021.getTempF());
  Serial.print(", Humidity: ");
  Serial.println(readings_Si7021.getHumidity());

  // MPL115A2
  readings_mpl115a2.setTempC(mpl115a2.getTemperature());
  readings_mpl115a2.setPressure(mpl115a2.getPressure());
  Serial.print("MPL115A: ");  
  Serial.print(readings_mpl115a2.getTempF());
  Serial.print(",                  Pressure: ");
  Serial.println(readings_mpl115a2.getPressure());

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

  float avgtemp = (readings_Si7021.getTempF() + readings_sht31.getTempF() + readings_ADT7410.getTempF() + readings_MCP9808.getTempF()) / 4;
  Serial.print("Average Temperature: ");
  Serial.println(avgtemp);
  Serial.println();

  displayParams(readings_SCD30.getTempF(), readings_SCD30.getHumidity(), readings_SCD30.getCO2());
  
  logData();
  delay(25000);
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
  readings_SCD30.setTempOffset(SCD30.getTemperatureOffset());
  Serial.print("    -- SCD30 Temperature offset: ");
  Serial.print(readings_SCD30.getTempOffset());
  Serial.println(" *C");  
}

void displayParams(float T, float H, float CO2) {

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
  display.print(CO2, 0); 

  // Line 3 - Temp
  display.setCursor (0, 32);
  display.print(T);
  display.print("F");

  // Line 4 - Humidity
  display.setCursor (0, 48);
  display.print(H);
  display.print("%");
  
  // Display everyting
  display.display(); 
}


void sdCardInit() {
  Serial.print("\nInitializing SD card...");

  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, chipSelect)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    while (1);
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  // print the type of card
  Serial.println();
  Serial.print("Card type:         ");
  switch (card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    while (1);
  }

  Serial.print("Clusters:          ");
  Serial.println(volume.clusterCount());
  Serial.print("Blocks x Cluster:  ");
  Serial.println(volume.blocksPerCluster());

  Serial.print("Total Blocks:      ");
  Serial.println(volume.blocksPerCluster() * volume.clusterCount());
  Serial.println();

  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("Volume type is:    FAT");
  Serial.println(volume.fatType(), DEC);

  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  volumesize /= 2;                           // SD card blocks are always 512 bytes (2 blocks are 1KB)
  Serial.print("Volume size (Kb):  ");
  Serial.println(volumesize);
  Serial.print("Volume size (Mb):  ");
  volumesize /= 1024;
  Serial.println(volumesize);
  Serial.print("Volume size (Gb):  ");
  Serial.println((float)volumesize / 1024.0);

  Serial.println("\nFiles found on the card (name, date and size in bytes): ");
  root.openRoot(volume);

  // list all files in the card with date and size
  root.ls(LS_R | LS_DATE | LS_SIZE);
}


void logData() {

  // Logging string
  String dataString = "";

  DateTime now = rtc.now();

  float arrayReadings[] = { readings_BME280.getTempF(), readings_BME280.getHumidity(), readings_BME280.getPressure(), readings_MCP9808.getTempF(), 
                          readings_ADT7410.getTempF(), readings_sht31.getTempF(), readings_sht31.getHumidity(), readings_Si7021.getTempF(), 
                          readings_Si7021.getHumidity(), readings_mpl115a2.getTempF(), readings_mpl115a2.getPressure(), readings_SCD30.getTempF(), 
                          readings_SCD30.getHumidity(), readings_SCD30.getCO2() };

 dataString += now.unixtime();
 dataString += ",";
 
 for (byte i = 0; i < 14; i = i+1) {

  //Serial.print(i);
  //Serial.print(": ");
  //Serial.println(arrayReadings[i]);
  dataString += arrayReadings[i];

  if ( i < 13 ) {
    dataString += ",";
    }
  }
  dataFile = SD.open("log2.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println("Data Logged to SD Card:");
    Serial.println(dataString);
    Serial.println();
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");  
 }
 
}
