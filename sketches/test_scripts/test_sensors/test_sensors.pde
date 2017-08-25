


float temp = 0;
int light = 0;
float vall = 0;



void setup()
{
  // Open the USB connection
  USB.ON();
  USB.println(F("Test"));


}

void loop()
{

  //  Utils.blinkGreenLED(500, 3); // blink 3 times during 500 ms each time

  // Show the remaining battery level
  USB.print(F("Battery Level: "));
  USB.print(PWR.getBatteryLevel(),DEC);
  USB.print(F(" %"));

  // Show the battery Volts
  USB.print(F(" | Battery (Volts): "));
  USB.print(PWR.getBatteryVolts());
  USB.println(F(" V"));
  delay(1000);

  // 3.3 volt sensors---------------------------------
  PWR.setSensorPower(SENS_3V3,SENS_ON); // Sets the 3,3V switch ON
  delay(1000);

  // Reading the DS1820 temperature sensor connected to DIGITAL8
  temp = Utils.readTempDS1820(DIGITAL8);
  USB.print(F("DS1820 Temperature: "));
  USB.print(temp);
  USB.println(F(" degrees"));
  delay(1000);

  // Reading Solar
  USB.print(F("Solar : "));
  USB.print(analogRead(ANALOG5));
  USB.println(F(" bit"));
  delay(1000);

  // Reading Distance 5 volt
  USB.print(F("Distance : "));
  USB.print(analogRead(ANALOG4)); //Distance sensor
  USB.println(F(" v"));
  delay(1000);

  // Reading Distance 5 volt
  USB.print(F("V+ : "));
  USB.print(analogRead(ANALOG7)); //3V3 out
  USB.println(F(" v"));
  delay(1000);

  PWR.setSensorPower(SENS_3V3,SENS_OFF); // Sets the 3,3V switch OFF
  delay(100);

  //delay(5000);
}











