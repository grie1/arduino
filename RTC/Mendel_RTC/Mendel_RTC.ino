/*
 *  MCP7940 RTC Test
 */

#include <MCP7940.h>   


const uint32_t SERIAL_SPEED        = 115200; // Set the baud rate for Serial I/O
const uint8_t  LED_PIN             =     13; // Arduino built-in LED pin number
const uint8_t  SPRINTF_BUFFER_SIZE =     32;   

char inputBuffer[SPRINTF_BUFFER_SIZE];   

MCP7940_Class MCP7940; // Create MCP7940

void setup() {
  Serial.begin(SERIAL_SPEED);
  while (!MCP7940.begin()) {
    Serial.println(F("Unable to find MCP7940M. Checking again in 3s."));
    delay(3000);
  }
  
  Serial.println("MCP7940 Found");
  
  while (!MCP7940.deviceStatus()) {
    Serial.println(F("Oscillator is off, turning it on."));
    bool deviceStatus = MCP7940.deviceStart();
    if (!deviceStatus) {
      Serial.println(F("Oscillator did not start, trying again."));
      delay(1000);
    }
  }

  Serial.println("Oscillator Found");

  pinMode(LED_PIN,OUTPUT);
  //MCP7940.adjust();
}

void loop() {                                                                 
  static uint8_t secs;
  DateTime now = MCP7940.now();
  Serial.print("The time is: ");
  sprintf(inputBuffer,"%04d-%02d-%02d %02d:%02d:%02d", now.year(),          
            now.month(), now.day(), now.hour(), now.minute(), now.second());  
  Serial.println(inputBuffer);                                                                                                    
  digitalWrite(LED_PIN,!digitalRead(LED_PIN)); 

  Serial.println();
  Serial.println("============= Register Settings =============");
  Serial.print("Battery Enabled (0 disabled): ");
  Serial.println(MCP7940.getBattery());
  MCP7940.setBattery(1);

  Serial.print("Power Fail Status: ");
  Serial.println(MCP7940.getPowerFail());
  now = MCP7940.getPowerDown();
  sprintf(inputBuffer,"xxxx-%02d-%02d %02d:%02d:xx",
            now.month(), now.day(), now.hour(), now.minute());   
  delay(3000);                                                             
}                                                       
