/*
  Juvass Temperature string and 4G
  2017-08-29 (C) john.hulth@geo.uio.no

*/


#include <Wasp4G.h>
// APN settings
///////////////////////////////////////
char apn[] = "telenor";
char login[] = "telenor";
char password[] = "telenor";
///////////////////////////////////////

// SERVER settings
///////////////////////////////////////
char ftp_server[] = "geo-ftp.uio.no";
uint16_t ftp_port = 21;
char ftp_user[] = "ftp";
char ftp_pass[] = "johnhh";
///////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Define filenames for SD card and FTP server:

char SERVER_FILE[40];
///////////////////////////////////////////////////////////////////////

// SMS settings
//////////////////////////////////////////////////
char phone_number[] = "41392555";
char sms_body[200];
//////////////////////////////////////////////////


// define variables
float batteryVoltage;
char batteryVoltage_str[10];
uint8_t batteryLevel;
bool chargeState;
uint16_t chargeCurrent;

// define file name: MUST be 8.3 SHORT FILE NAME
char filename[13];
char filename_old[13];
char timestamp[20];
uint8_t sd_answer;

uint8_t error;

uint8_t connection_status;
char operator_name[20];

uint32_t previous;

uint8_t gps_status;
float gps_latitude;
float gps_longitude;
bool gps_autonomous_needed = true;

float temp = 0;
float temp_air = 0;
char temp_air_str[10];
int count = 0;

void setup()
{
  // Create file, update time, setup 4G, GPS and FTP, send data,
  Send_etc();
}


void loop()
{
  USB.println(F("enter deep sleep"));
  // Go to sleep disconnecting all switches and modules
  // Waspmote wakes up thanks to the RTC Alarm every hour (MODE4)
  PWR.deepSleep("00:00:00:00", RTC_ABSOLUTE, RTC_ALM1_MODE4, ALL_OFF);

  USB.ON();
  RTC.ON();

  USB.println(F("\nwake up"));

  USB.println(RTC.getTime());

  // After wake up check interruption source
  if ( intFlag & RTC_INT )
  {
    // clear interruption flag
    intFlag &= ~(RTC_INT);

    USB.println(F("---------------------"));
    USB.println(F("RTC INT captured"));
    USB.println(F("---------------------"));
  }

  // if day 2 (Monday at 12) make a new filename and send data ('old file')
  if (RTC.day == 2 && RTC.hour == 12)
  {
    // Create new file, update time, setup 4G, GPS and FTP, send data,
    Send_etc();
  }

  // Take sample every 4 hour
  if (RTC.hour == 0 || RTC.hour == 4 || RTC.hour == 8 || RTC.hour == 12 || RTC.hour == 16 || RTC.hour == 20)
  {
    USB.println("-------------------------------------");
    PWR.setSensorPower(SENS_3V3, SENS_ON);
    delay(100);

    SD.ON();

    // Add timestamp in file
    sprintf(timestamp, "%02u%02u%02u,%02u%02u, ", RTC.year, RTC.month, RTC.date, RTC.hour, RTC.minute);
    SD.append(filename, timestamp);

    // Air temperature is also on the string-board for now
    //    // Read air temperature connected to DIGITAL6
    //    temp_air = Utils.readTempDS1820(DIGITAL6);
    //    USB.print(temp_air);
    //    dtostrf( temp, 2, 3, temp_air_str);
    //    SD.append(filename, temp_air_str);
    //    SD.append(filename, ", ");

    // Reading the DS1820 temperature sensors chain connected to DIGITAL8
    temp = readTempDS1820chain(DIGITAL8,  true);
  }
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
  char temp_str[10];
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
    SD.append(filename, addr);
    SD.append(filename, ": ");

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

    dtostrf( temp, 2, 3, temp_str);

    SD.append(filename, temp_str);
    SD.append(filename, ", ");

    USB.println("");
  }

  // USB.println("-------------- No more addresses --------------");

  SD.append(filename, "\n");

  OneWireTemp.reset_search();
  delay(250);

  OneWireTemp.reset_search();

  return temp;
}

void Send_etc(void)
{
  USB.ON();
  SD.ON();
  USB.println("Start program");

  // General stuff--------------------------------------------------
  /////////////////////////////////////////////////////////////
  // 1. Create file according to DATE with the following format:
  // filename: [YYMMDD.TXT]
  /////////////////////////////////////////////////////////////
  sprintf(filename, "%02u%02u%02u.TXT", RTC.year, RTC.month, RTC.date);
  if (SD.create(filename))
  {
    USB.print(F("1 - file created:"));
    USB.println(filename);
  }
  else
  {
    USB.println(F("1 - file NOT created")); // only one file per day
  }

  sprintf(filename_old, "%s", filename); // this should move up before 'make filename'!!!!!!!

  USB.println(filename);
  USB.println(filename_old);



  // get battery volt and level
  batteryVoltage = PWR.getBatteryVolts();
  dtostrf( batteryVoltage, 1, 2, batteryVoltage_str);

  batteryLevel = PWR.getBatteryLevel();

  // get charging state and current
  chargeState = PWR.getChargingState();
  chargeCurrent = PWR.getBatteryCurrent();



  // GPS and 4G stuff--------------------------------------------------
  // error = GPSand4G();

  //////////////////////////////////////////////////
  // Set operator parameters
  //////////////////////////////////////////////////
  _4G.set_APN(apn, login, password);

  //////////////////////////////////////////////////
  // Show APN settings via USB port
  //////////////////////////////////////////////////
  _4G.show_APN();


  //////////////////////////////////////////////////
  // 1. Switch ON the 4G module
  //////////////////////////////////////////////////
  error = _4G.ON();

  if (error == 0)
  {
    USB.println(F("1. 4G module ready"));


    ////////////////////////////////////////////////
    // Enter PIN code
    ////////////////////////////////////////////////
    USB.println(F("Setting PIN code..."));
    if (_4G.enterPIN("9943") == 0)
    {
      USB.println(F("PIN code accepted"));
    }
    else
    {
      USB.println(F("PIN code incorrect"));
    }

    delay(1000);

    ////////////////////////////////////////////////
    // 1.1. Check connection to network and continue
    ////////////////////////////////////////////////
    connection_status = _4G.checkDataConnection(30);

    if (connection_status == 0)
    {
      USB.println(F("1.1. Module connected to network"));

      // delay for network parameters stabilization
      delay(5000);

      //////////////////////////////////////////////
      // 1.2. Get RSSI
      //////////////////////////////////////////////
      error = _4G.getRSSI();
      if (error == 0)
      {
        USB.print(F("1.2. RSSI: "));
        USB.print(_4G._rssi, DEC);
        USB.println(F(" dBm"));
      }
      else
      {
        USB.println(F("1.2. Error calling 'getRSSI' function"));
      }

      //////////////////////////////////////////////
      // 1.3. Get Network Type
      //////////////////////////////////////////////
      error = _4G.getNetworkType();

      if (error == 0)
      {
        USB.print(F("1.3. Network type: "));
        switch (_4G._networkType)
        {
          case Wasp4G::NETWORK_GPRS:
            USB.println(F("GPRS"));
            break;
          case Wasp4G::NETWORK_EGPRS:
            USB.println(F("EGPRS"));
            break;
          case Wasp4G::NETWORK_WCDMA:
            USB.println(F("WCDMA"));
            break;
          case Wasp4G::NETWORK_HSDPA:
            USB.println(F("HSDPA"));
            break;
          case Wasp4G::NETWORK_LTE:
            USB.println(F("LTE"));
            break;
          case Wasp4G::NETWORK_UNKNOWN:
            USB.println(F("Unknown or not registered"));
            break;
        }
      }
      else
      {
        USB.println(F("1.3. Error calling 'getNetworkType' function"));
      }

      //////////////////////////////////////////////
      // 1.4. Get Operator name
      //////////////////////////////////////////////
      memset(operator_name, '\0', sizeof(operator_name));
      error = _4G.getOperator(operator_name);

      if (error == 0)
      {
        USB.print(F("1.4. Operator: "));
        USB.println(operator_name);
      }
      else
      {
        USB.println(F("1.4. Error calling 'getOperator' function"));
      }
    }
  }
  else
  {
    // Problem with the communication with the 4G module
    USB.println(F("4G module not started"));
    USB.print(F("Error code: "));
    USB.println(error, DEC);
  }

  ////////////////////////////////////////////////
  // 2.1. FTP open session, send data in filename_old
  ////////////////////////////////////////////////

  error = _4G.ftpOpenSession(ftp_server, ftp_port, ftp_user, ftp_pass);

  // check answer
  if (error == 0)
  {
    USB.println(F("2.1. FTP open session OK"));

    previous = millis();

    //////////////////////////////////////////////
    // 2.2. FTP upload
    //////////////////////////////////////////////

    snprintf(SERVER_FILE, sizeof(SERVER_FILE), "/incoming/%s", filename_old );
    USB.println(SERVER_FILE);


    error = _4G.ftpUpload(SERVER_FILE, filename_old);

    if (error == 0)
    {

      USB.print(F("2.2. Uploading SD file to FTP server done! "));
      USB.print(F("Upload time: "));
      USB.print((millis() - previous) / 1000, DEC);
      USB.println(F(" s"));
    }
    else
    {
      USB.print(F("2.2. Error calling 'ftpUpload' function. Error: "));
      USB.println(error, DEC);
    }


    //////////////////////////////////////////////
    // 2.3. FTP close session
    //////////////////////////////////////////////

    error = _4G.ftpCloseSession();

    if (error == 0)
    {
      USB.println(F("2.3. FTP close session OK"));
    }
    else
    {
      USB.print(F("2.3. Error calling 'ftpCloseSession' function. error: "));
      USB.println(error, DEC);
      USB.print(F("CMEE error: "));
      USB.println(_4G._errorCode, DEC);
    }
  }
  else
  {
    USB.print(F( "2.1. FTP connection error: "));
    USB.println(error, DEC);
  }



  ////////////////////////////////////////////////
  // 2. Start GPS feature
  ////////////////////////////////////////////////
  if (connection_status == 0)
  {
    USB.println(F("Starting GPS..."));
    // get current time
    previous = millis();

    gps_status = _4G.gpsStart(Wasp4G::GPS_MS_BASED);

    // check answer
    if (gps_status == 0)
    {
      USB.print(F("2. GPS started in MS-BASED. Time(secs) = "));
      USB.println((millis() - previous) / 1000);
    }
    else
    {
      USB.print(F("2. Error calling the 'gpsStart' function. Code: "));
      USB.println(gps_status, DEC);
    }
  }

  ////////////////////////////////////////////////
  // Wait for satellite signals and get values
  ////////////////////////////////////////////////
  if (gps_status == 0)
  {
    error = _4G.waitForSignal(90000);

    if (error == 0)
    {
      USB.print(F("3. GPS signal received. Time(secs) = "));
      USB.println((millis() - previous) / 1000);

      USB.println(F("Acquired position:"));
      USB.println(F("----------------------------"));
      USB.print(F("Latitude: "));
      USB.print(_4G._latitude);
      USB.print(F(","));
      USB.println(_4G._latitudeNS);
      USB.print(F("Longitude: "));
      USB.print(_4G._longitude);
      USB.print(F(","));
      USB.println(_4G._longitudeEW);
      USB.print(F("UTC_time: "));
      USB.println(_4G._time);
      USB.print(F("date: "));
      USB.println(_4G._date);
      USB.print(F("Number of satellites: "));
      USB.println(_4G._numSatellites, DEC);
      USB.print(F("HDOP: "));
      USB.println(_4G._hdop);
      USB.println(F("----------------------------"));

      // get degrees
      gps_latitude  = _4G.convert2Degrees(_4G._latitude, _4G._latitudeNS);
      gps_longitude = _4G.convert2Degrees(_4G._longitude, _4G._longitudeEW);

      USB.println("Conversion to degrees:");
      USB.print(F("Latitude: "));
      USB.println(gps_latitude);
      USB.print(F("Longitude: "));
      USB.println(gps_longitude);
      USB.println();
    }
    else
    {
      USB.print("no satellites fixed. Error: ");
      USB.println(error, DEC);
    }
  }

  ////////////////////////////////////////////////
  // Set RTC time
  ////////////////////////////////////////////////
  USB.print(F("Time [Day of week, YY/MM/DD, hh:mm:ss]: "));
  USB.println(RTC.getTime());

  if (connection_status == 0 && gps_status == 0 && error == 0)
  {
    error = _4G.setTimeFrom4G();
    if (error == 0)
    {
      USB.print(F("Time set to [Day of week, YY/MM/DD, hh:mm:ss]: "));
    }
    else
    {
      USB.print(F("Time NOT! changed [Day of week, YY/MM/DD, hh:mm:ss]: "));
    }
    //Show time

    USB.println(RTC.getTime());
  }

  // Send  confirmation sms
  if (connection_status == 0)
  {
    snprintf( sms_body, sizeof(sms_body), "%s Volt, Batt_level: %d , Charging: %d, %d mA, filename: %s", batteryVoltage_str, batteryLevel, chargeState, chargeCurrent, filename);


    // snprintf( sms_body, sizeof(sms_body), "A:%c#B:%d#C:%s#D:%lu#E:%x#F:%s#", character, integer, string, ulong, character, float_str );


    USB.println(sms_body);

    ////////////////////////////////////////////////
    // 2. Configure SMS options
    ////////////////////////////////////////////////

    if (connection_status == 0 && gps_status == 0 && error == 0)
    { error = _4G.configureSMS();

      if (error == 0)
      {
        USB.println(F("2.1. 4G module configured to use SMS"));
      }
      else
      {
        USB.print(F("2.1. Error calling 'configureSMS' function. Code: "));
        USB.println(error, DEC);
      }


      ////////////////////////////////////////////////
      // 3. Send SMS
      ////////////////////////////////////////////////
      USB.print(F("2.2. Sending SMS..."));
      error = _4G.sendSMS( phone_number, sms_body);

      if (error == 0)
      {
        USB.println(F("done"));
      }
      else
      {
        USB.print(F("error. Code: "));
        USB.println(error, DEC);
      }
    }
  }
}








