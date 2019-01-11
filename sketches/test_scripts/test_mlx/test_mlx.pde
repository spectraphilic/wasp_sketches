//#include "WaspUIO.h"
#include <SparkFunMLX90614.h> // SparkFunMLX90614 Arduino library

IRTherm therm; // Create an IRTherm object to interact with throughout
float object ;
float ambient;


void setup() 
{
  PWR.setSensorPower(SENS_3V3, SENS_ON);
  delay(100);

USB.println((long)I2C.scan(0x68));
  USB.println((long)I2C.scan(0x5A));

  therm.begin(); // Initialize thermal IR sensor
  therm.setUnit(TEMP_C); 
  
 if (therm.readID()) // Read from the ID registers
  { // If the read succeeded, print the ID:
    USB.print("ID: 0x");
    USB.print(therm.getIDH());
    USB.println(therm.getIDL());
  }

  USB.println(F("setup done"));
  
}

void loop() 
{
PWR.setSensorPower(SENS_3V3, SENS_ON);
  therm.read();
  object = therm.object();
  ambient = therm.ambient();

  // Debug
  char str[20];
 USB.print(F("\nMLX 90614 object= "));
 USB.println(Utils.float2String(object, str, 2));
  USB.print(F("MLX 90614 ambient= "));
  USB.println(Utils.float2String(ambient, str, 2));
  delay(1000);
}

