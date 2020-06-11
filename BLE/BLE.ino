/*
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updates by chegewara
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include "Adafruit_HTU21DF.h"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define TEMPERATURE_UUID    "9f65e9b2-2147-49fa-b096-2eb94b05f01b"
#define HUMIDITY_UUID       "410a8735-cb6e-40ac-83c2-fa0e7e5cc77d"

BLECharacteristic *pCharacteristic; // A String
BLECharacteristic *pCharacteristic2; // Temp
BLECharacteristic *pCharacteristic3;  // Humidity

Adafruit_HTU21DF htu = Adafruit_HTU21DF();

void setup() {
  Serial.begin(115200);

  // Init Temp Sensor
  Serial.println("Init temp sensor");
  if (!htu.begin()) {
    Serial.println("Couldn't find sensor!");
    while (1);
  }
  
  Serial.println("Starting BLE");

  // Init BLE Environment
  BLEDevice::init("ESP32 BLE Example Server");

  // Create BLE Server
  BLEServer *pServer = BLEDevice::createServer();

  // Create BLE Service (owned by server)
  BLEService *pService = pServer->createService(SERVICE_UUID);
  
  // Create BLE Characteristic 1 (owned by service 1)
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  // Set value of characteristic
  pCharacteristic->setValue("Hello World says Eric");
  
  pCharacteristic2 = pService->createCharacteristic(
                                         TEMPERATURE_UUID,
                                         BLECharacteristic::PROPERTY_READ|
                                         BLECharacteristic::PROPERTY_NOTIFY|
                                         BLECharacteristic::PROPERTY_INDICATE
                                       );
  pCharacteristic2->setValue("0.00");

  pCharacteristic3 = pService->createCharacteristic(
                                         HUMIDITY_UUID,
                                         BLECharacteristic::PROPERTY_READ|
                                         BLECharacteristic::PROPERTY_NOTIFY|
                                         BLECharacteristic::PROPERTY_INDICATE
                                       );
  pCharacteristic3->setValue("0.00");

  // Start our service
  pService->start();
  
  // BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  
  // Start Advertising our Services
  BLEDevice::startAdvertising();
  Serial.println("Characteristics defined BLE Advertising Start");
}

void loop() {
  float temp = htu.readTemperature();
  float rel_hum = htu.readHumidity();
  Serial.print("Temp: "); Serial.print(temp); Serial.print(" C");
  Serial.print("\t\t");
  Serial.print("Humidity: "); Serial.print(rel_hum); Serial.println(" \%");

  // Update BLE Temp
  //pCharacteristic->setValue("Hello World says Eric");
  Serial.println("Update BLE Temp & Humidity Characteristic");
  pCharacteristic2->setValue(temp);
  pCharacteristic3->setValue(rel_hum);
   
  delay(2000);
}
