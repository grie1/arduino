#include <Wire.h>

#include "SparkFun_SCD30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD30
SCD30 SCD30;

void setup()
{
  Serial.begin(115200);
  Serial.println("SCD30 Example");
  Wire.begin();

  if (SCD30.begin() == false)
  {
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    while (1)
      ;
  }

  //The SCD30 has data ready every two seconds
}

void loop()
{
  if (SCD30.dataAvailable())
  {
    Serial.print("co2(ppm):");
    Serial.print(SCD30.getCO2());

    Serial.print(" temp(C):");
    Serial.print(SCD30.getTemperature(), 1);

    Serial.print(" humidity(%):");
    Serial.print(SCD30.getHumidity(), 1);

    Serial.println();
  }
  else
    Serial.println("Waiting for new data");

  delay(2000);
}
