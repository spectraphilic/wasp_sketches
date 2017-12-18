/*
    ------ Waspmote reading DS1820 temperature sensor chain --------
   Working ok /JH 2017-12-18




*/

// Variables

float temp = 0;
int count = 0;

void setup()
{
  // Init USB
  USB.ON();
  RTC.ON();
  delay(10);

}

void loop()
{
  USB.println("-------------------------------------");
  PWR.setSensorPower(SENS_3V3, SENS_ON);

  delay(100);

  // Reading the DS1820 temperature sensor connected to DIGITAL6
  temp = readTempDS1820chain(DIGITAL6,  true);

  delay(100);
}

// - - - - - FUNCTION - - - - -

/* readTempDS1820() - reads the DS1820 temperature sensor

   It reads the DS1820 temperature sensor
*/
float readTempDS1820chain(uint8_t pin, bool is3v3 )
{
  // check if it is necessary to turn on
  // the generic 3v3 power supply
  if ( is3v3 == true )
  {
    PWR.setSensorPower(SENS_3V3, SENS_ON);

    delay(10);
  }

  WaspOneWire OneWireTemp(pin);
  byte i;  // JH !!!
  byte present = 0; // JH !!!
  byte data[12];
  byte addr[8];
  byte maxsensors = 0;
  // float temp = 0;
  uint32_t R_bin;


  while (OneWireTemp.search(addr))
  {
    // USB.print("R="); // Print serial number here...
    for ( i = 1; i < 7; i++)
    {
      if (addr[i] < 16) {
        USB.print('0');
      }
      USB.print(addr[i], HEX);
      if (i < 6) {
        USB.print(" ");
      }
      else {
        USB.print(" ");
      }
    }

    if ( WaspOneWire::crc8( addr, 7) != addr[7]) {
      USB.print("CRC is not valid!\n");
      return -1000;
    }

    if ( addr[0] == 0x10) {
      USB.print("Device is a DS18S20 family device.\n");
      maxsensors++;
    }
    else {
      if (addr[0] == 0x28) {
        // USB.print("Device is a DS18B20 family device.\n");
        maxsensors++;
      }
      else {
        USB.print("Device is unknown!\n");
        USB.print("Device ID: ");
        USB.print(addr[0], HEX);
        USB.println();
        return -1000;
      }
    }
    // The DallasTemperature library can do all this work for you!
    OneWireTemp.reset();
    OneWireTemp.select(addr);
    OneWireTemp.write(0x44, 1);        // start conversion, with parasite power on at the end
    delay(1000);                       // maybe 750ms is enough, maybe not
    // we might do a OneWireTemp.depower() here, but the reset will take care of it.
    present = OneWireTemp.reset();
    OneWireTemp.select(addr);
    OneWireTemp.write(0xBE);         // Read Scratchpad

    for ( i = 0; i < 9; i++) {           // we need 9 bytes
      data[i] = OneWireTemp.read();

    }

    // Print temperature
    byte MSB = data[1];
    byte LSB = data[0];

    float tempRead = ((MSB << 8) | LSB); //using two's compliment
    temp = tempRead / 16;

    USB.print("T: ");
    USB.println(temp);
    USB.println("");
  }

  // USB.println("-------------- No more addresses --------------");
  OneWireTemp.reset_search();
  delay(250);

  OneWireTemp.reset_search();

  return temp;
}










