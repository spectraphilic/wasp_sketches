/*
    ------ Waspmote reading DS1820 temperature sensor chain --------
   Working ok /JH
*/

// Variables

float temp = 0;
int count = 0;

void setup()
{
  // Init USB
  USB.ON();

  PWR.setSensorPower(SENS_3V3, SENS_ON);
  delay(10);

  //  // set DIGITAL pin 1-6 as output and set them LOW, digital 8 is used for OneWire comunication
  //  pinMode(DIGITAL1 , OUTPUT);
  //  pinMode(DIGITAL2 , OUTPUT);
  //  pinMode(DIGITAL3 , OUTPUT);
  //  pinMode(DIGITAL4 , OUTPUT);
  //  pinMode(DIGITAL5 , OUTPUT);
  //  pinMode(DIGITAL6 , OUTPUT);
  //
  //  digitalWrite(DIGITAL1, LOW);
  //  digitalWrite(DIGITAL2, LOW);
  //  digitalWrite(DIGITAL3, LOW);
  //  digitalWrite(DIGITAL4, LOW);
  //  digitalWrite(DIGITAL5, LOW);
  //  digitalWrite(DIGITAL6, LOW);
}

void loop()
{
  //  // Reset count
  //  count = 0;
  //
  //  // for ( int x = 0; x < 8; x++) // MUX_GND
  //  for ( int x = 0; x < 2; x++) // MUX_GND
  //  {
  //    int r0 = x & 0x01;           // Mux A-H, bit 1
  //    int r1 = (x >> 1) & 0x01;    // Mux A-H, bit 2
  //    int r2 = (x >> 2) & 0x01;    // Mux A-H, bit 3
  //
  //    // digitalWrite(DIGITAL1, LOW);
  //    // digitalWrite(DIGITAL1, HIGH);
  //    // digitalWrite(DIGITAL2, LOW);
  //    // digitalWrite(DIGITAL3, LOW);
  //    // digitalWrite(DIGITAL3, HIGH);
  //
  //    digitalWrite(DIGITAL1, r0);
  //    digitalWrite(DIGITAL2, r1);
  //    digitalWrite(DIGITAL3, r2);
  //
  //
  //    for ( byte y = 0; y < 8; y++) // MUX_DQ
  //    {
  //      int s0 = y & 0x01;           // Mux 1-8, bit 1
  //      int s1 = (y >> 1) & 0x01;    // Mux 1-8, bit 2
  //      int s2 = (y >> 2) & 0x01;    // Mux 1-8, bit 3
  //
  //      // digitalWrite(DIGITAL4, LOW);
  //      // digitalWrite(DIGITAL4, HIGH);
  //      // digitalWrite(DIGITAL5, LOW);
  //      // digitalWrite(DIGITAL6, LOW);
  //      // digitalWrite(DIGITAL6, HIGH);
  //
  //      digitalWrite(DIGITAL4, s0);
  //      digitalWrite(DIGITAL5, s1);
  //      digitalWrite(DIGITAL6, s2);
  //
  USB.println("-------------------------------------");
  //
  //      count++;
  //      USB.printf("Temp%u\n", count);
  //
  //      USB.println("      MUX_GND    MUX_DQ");
  //      USB.println("      r2 r1 r0   s2 s1 s0");
  //      USB.printf("Bit   %u  %u  %u    %u  %u  %u\n", r2, r1, r0, s2, s1, s0);

  delay(200);

  // Reading the DS1820 temperature sensor connected to DIGITAL8
  temp = readTempDS1820chain(DIGITAL8,  true);
  //  USB.print(F("DS1820 Temperature: "));
  //  USB.print(temp);
  //  USB.println(F(" degrees"));
  delay(1000);
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
        // USB.println("");
      }
      // R_bin = (addr[i] << ((i - 2) * 8));  //????????? how to get binary string???
    }
    // USB.println(R_bin, BIN);

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
    //    USB.print("P=");
    //    USB.print(present, HEX);
    //    USB.print(" ");
    for ( i = 0; i < 9; i++) {           // we need 9 bytes
      data[i] = OneWireTemp.read();
      //      USB.print(data[i], HEX);
      //      USB.print(" ");
    }
    //    USB.print(" CRC=");
    //    USB.print( WaspOneWire::crc8( data, 8), HEX);
    //    USB.println();


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

  // delay(1000);


  OneWireTemp.reset_search();

  // check if it is necessary to turn off
  // the generic 3v3 power supply
  // if ( is3v3 == true )
  //  {
  //    PWR.setSensorPower(SENS_3V3, SENS_OFF);
  //  }

  return temp;
}










