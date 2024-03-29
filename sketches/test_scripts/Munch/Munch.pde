/*
  Munch Temperature, wind and 3G
  2017-10-22 (C) john.hulth@geo.uio.no
*/

#include <Wasp3G.h>
#include <SDI12.h>

// APN settings
///////////////////////////////////////
char apn[] = "telenor";
char login[] = "telenor";
char password[] = "telenor";
///////////////////////////////////////

char PIN_Code[] = "1031";

// SERVER settings
///////////////////////////////////////
#define libelium 1 // set to 1 for libelium-ftp and 0 for geo-ftp

#if libelium
// pruebas.libelium.com/FOLDER2/
char ftp_server[] = "pruebas.libelium.com";
char ftp_port[] = "21";
char ftp_user[] = "t3g@libelium.com";
char ftp_pass[] = "ftp1234";

char path[] = "/test";

#else
// geo-ftp.uio.no/incoming/Munch/
char ftp_server[] = "geo-ftp.uio.no";
char ftp_port[] = "21";
char ftp_user[] = "ftp";
char ftp_pass[] = "Munch";

char path[] = "/incoming";
#endif
///////////////////////////////////////

///////////////////////////////////////////////////////////////////////
// Define filenames for SD card and FTP server:

//char SERVER_FILE[40];
///////////////////////////////////////////////////////////////////////

// SMS settings
//////////////////////////////////////////////////
char phone_number[] = "41392555";
char sms_body[120]; // Max 160 bytes for SMS
//////////////////////////////////////////////////


// define variables
float batteryVoltage;
char batteryVoltage_str[10];
uint8_t batteryLevel;
bool chargeState;
uint16_t chargeCurrent;

// define file name: MUST be 8.3 SHORT FILE NAME
char filename[50] = "000000.txt";
char filename_old[50];
char timestamp[20];
uint8_t sd_answer;

uint8_t error;
int8_t answer;
int8_t GPS_status;
uint8_t connection_status;
char operator_name[20];

uint32_t previous;

float gps_latitude;
float gps_longitude;
bool gps_autonomous_needed = true;

//DS-2
#define DATAPIN DIGITAL8         // change to the proper pin (JH) 6 = DIGITAL 6 on Waspmote
SDI12 mySDI12(DATAPIN);
char sdiResponse[30];
char cmd[10];

char sdi_adress[] = "abc?"; // SDI-Adresses for the sensors in use, ex. "abc","?", "12345", "ABCD" or "aB1bC2"

// DS18B20
float temp = 0;
float temp_air = 0;
char temp_air_str[10];
// int count = 0;

void setup()
{
  // Create file, update time, setup 3G, GPS and FTP, send data,
  Send_etc();

  // Setup DS-2

  USB.ON();
  // PWR.setSensorPower(SENS_5V, SENS_ON); // (JH)
  delay(500); // Sensor exitation delay

  //  mySDI12.begin();
  //
  //  // -'-'-'-'-'-'-INFO COMMAND-'-'-'-'-'-'-
  //  for (int i = 0; i < (sizeof(sdi_adress) - 1) ; i++)
  //  {
  //    snprintf( cmd, sizeof(cmd), "%cI!", sdi_adress[i] ); // Info command (?I!)
  //    SDIdata(cmd);
  //
  //    snprintf(cmd, sizeof(cmd), "%cM6!", sdi_adress[i]); // Start measurments command (?M6!) DS-2
  //    SDIdata(cmd);
  //  }
  //  USB.println("- - - - - - - - - - - - - - - -");


}


void loop()
{
  USB.println(F("enter deep sleep"));
  // Go to sleep disconnecting all switches and modules
  // Waspmote wakes up thanks to the RTC Alarm every hour (MODE4)
  // Waspmote wakes up thanks to the RTC Alarm every minute (MODE5)

  RTC.setWatchdog(10); // Re-boot after 10 min
  //  PWR.deepSleep("00:00:00:00", RTC_ABSOLUTE, RTC_ALM1_MODE5, ALL_ON); // Wake-up every 1 min
  PWR.deepSleep("00:00:00:00", RTC_ABSOLUTE, RTC_ALM1_MODE5, ALL_OFF); // Wake-up every 1 min

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

  //  make a new filename and send data once a day ('old file')
  //  if (RTC.hour == 0 && RTC.minute == 0)

  //  make a new filename and send data once an hour ('old file')
  if (RTC.minute == 0)

  {
    // Create new file, update time, setup 3G, GPS and FTP, send data,
    Send_etc();
  }

  // Take sample every wake-up (minute?)

  USB.println("----start measurement-------------------------");

  SD.ON();

  //  // Reading DS-2 wind sensor
  //  for (int i = 0; i < (sizeof(sdi_adress) - 1) ; i++)
  //  {
  //    snprintf(cmd, sizeof(cmd), "%cR6!", sdi_adress[i]); // Continuous measurments command (?R6!) DS-2
  //    SDIdata(cmd);
  //    USB.println(sdi_adress[i]);
  //  }

  // Reading the DS1820 temperature sensors chain connected to DIGITAL8
  // PWR.setSensorPower(SENS_3V3, SENS_ON);
  // delay(1000);

  temp = readTempDS1820chain(DIGITAL8,  true);

  PWR.setSensorPower(SENS_3V3, SENS_OFF);

  // Power off SD card
  // SD.OFF();
}







// - - - - - FUNCTION - - - - -


/* readTempDS1820() - reads the DS1820 temperature sensor

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
  char serial[18];
  //byte maxsensors = 0;
  //uint32_t R_bin;
  char sample_txt[100];
  char temp_str[7];


  while (OneWireTemp.search(addr))
  {
    // Print time
    RTC.getTime();
    // USB.print(RTC.getTime());

    // USB.print("R="); // Print serial number here...
    snprintf(serial, sizeof(serial), "%02X %02X %02X %02X %02X %02X", addr[1], addr[2], addr[3], addr[4], addr[5], addr[6] );
    // USB.print(serial);

    // Check CRC and Family code
    if ( WaspOneWire::crc8( addr, 7) != addr[7])
    {
      USB.print("CRC is not valid!\n");
      return -1000;
    }

    // Check Family code
    if (addr[0] == 0x28)
    {
      // USB.print("Device is a DS18B20 family device.\n");
    }
    else
    {
      USB.print("Device is unknown!\n");
      USB.print("Device ID: ");
      USB.print(addr[0], HEX);
      USB.println();
      return -1000;
    }

    // The DallasTemperature library can do all this work for you!
    OneWireTemp.reset();
    OneWireTemp.select(addr);
    OneWireTemp.write(0x44, 1);        // start conversion, with parasite power on at the end
    delay(930);                       // maybe 750ms is enough, maybe not

    // we might do a OneWireTemp.depower() here, but the reset will take care of it.
    present = OneWireTemp.reset();
    OneWireTemp.select(addr);
    OneWireTemp.write(0xBE);         // Read Scratchpad

    for ( i = 0; i < 9; i++) // we need 9 bytes
    {
      data[i] = OneWireTemp.read();
    }

    // Print temperature
    byte MSB = data[1];
    byte LSB = data[0];

    float tempRead = ((MSB << 8) | LSB); //using two's compliment
    temp = tempRead / 16;

    dtostrf(temp, 2, 4, temp_str);

    // USB.print("T: ");
    // USB.println(temp_str);

    memset(sample_txt, 0, sizeof(sample_txt));
    snprintf(sample_txt, sizeof(sample_txt), "20%02u-%02u-%02u %02u:%02u:%02u, %s, T:%s", RTC.year, RTC.month, RTC.date, RTC.hour, RTC.minute, RTC.second, serial, temp_str);

    USB.println(sample_txt);

    SD.appendln(filename, sample_txt);

  }

  // USB.println("-------------- No more addresses --------------");
  OneWireTemp.reset_search();
  delay(250);
  // OneWireTemp.reset_search();
  return temp;
}


// Function to read data from SDI-12 sensor
void SDIdata(char* cmd)
{
  char sample_txt[100];

  mySDI12.sendCommand(cmd);

  int i = 0;
  while (mySDI12.available())  // write the response to the screen
  {
    char c = mySDI12.read();
    if ((c != '\n') && (c != '\r'))
    {
      sdiResponse[i] = c;
      i++;
    }
    delay(5);
  }

  // USB.println(sdiResponse); //write the response to the screen
  // SD.appendln(filename, sdiResponse); //write the response to SD-card

  // Print time
  RTC.getTime();
  // USB.print(RTC.getTime());

  memset(sample_txt, 0, sizeof(sample_txt));
  snprintf(sample_txt, sizeof(sample_txt), "20%02u-%02u-%02u %02u:%02u:%02u, %s", RTC.year, RTC.month, RTC.date, RTC.hour, RTC.minute, RTC.second, sdiResponse);

  USB.println(sample_txt);

  SD.appendln(filename, sample_txt);

  delay(1000); // Needed for CTD10 pressure sensor
  mySDI12.flush();
  memset (sdiResponse, 0, sizeof(sdiResponse));
}


// Make file and send data 3G functions
void Send_etc(void)
{
  USB.ON();
  SD.ON();
  // USB.println("Start program");

  // General stuff--------------------------------------------------
  /////////////////////////////////////////////////////////////
  // 1. Create file according to DATE with the following format:
  // filename: [YYMMDDHH.TXT]
  /////////////////////////////////////////////////////////////

  // Rename old_filename...
  sprintf(filename_old, "%s", filename);

  sprintf(filename, "%s/%02u%02u%02u%02u.TXT",  path, RTC.year, RTC.month, RTC.date, RTC.hour);

  if (SD.mkdir(path)) // Make directory
  {
    USB.println(F("directories created OK"));
  }
  else
  {
    USB.println(F("mkdir failed"));
  }

  if (SD.create(filename)) // Create new file
  {
    USB.print(F("1 - file created:"));
    USB.println(filename);
    SD.appendln(filename, filename);
  }
  else
  {
    USB.println(F("1 - file NOT created")); // only one file per day
  }


  USB.print(F("filename: "));
  USB.println(filename);
  USB.print(F("filename_old: "));
  USB.println(filename_old);



  // get battery volt and level
  batteryVoltage = PWR.getBatteryVolts();
  dtostrf( batteryVoltage, 1, 2, batteryVoltage_str);

  batteryLevel = PWR.getBatteryLevel();

  // get charging state and current
  chargeState = PWR.getChargingState();
  chargeCurrent = PWR.getBatteryCurrent();


  snprintf( sms_body, sizeof(sms_body), "Munch, %s Volt, Batt_level: %d, Charging: %d, %d mA, filename: %s", batteryVoltage_str, batteryLevel, chargeState, chargeCurrent, filename);
  USB.println(sms_body);

  if (SD.appendln(filename, sms_body))
  {
    USB.println("Info appanded to file, OK!");
  }
  else
  {
    USB.println("Appanded info to file, FAILED!");
  }

  // GPS and 3G stuff--------------------------------------------------
  // error = GPSand3G();

  //////////////////////////////////////////////////
  // Set operator parameters
  //////////////////////////////////////////////////
  _3G.set_APN(apn, login, password);

  //////////////////////////////////////////////////
  // Show APN settings via USB port
  //////////////////////////////////////////////////
  _3G.show_APN();


  //////////////////////////////////////////////////
  // 1. Switch ON the 3G module
  //////////////////////////////////////////////////
  answer = _3G.ON();
  if ((answer == 1) || (answer == -3))
  {
    USB.println(F("1. 3G module ready"));


    ////////////////////////////////////////////////
    // Enter PIN code
    ////////////////////////////////////////////////
    USB.println(F("Setting PIN code..."));
    // if (_4G.enterPIN(PIN_Code) == 0)
    if (_3G.setPIN(PIN_Code) == 1)
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
    connection_status = _3G.check(60);

    if (connection_status == 1)
    {
      USB.println(F("1.1. Module connected to network"));

      // delay for network parameters stabilization
      delay(5000);


      //////////////////////////////////////////////
      // 1.2. Get RSSI
      //////////////////////////////////////////////
      answer = _3G.getRSSI();
      if (answer != 0)
      {
        USB.print(F("RSSI: "));
        USB.print(answer, DEC);
        USB.println(F("dBm"));
      }
      else
      {
        USB.println(F("1.2. Error calling 'getRSSI' function"));
      }

      //////////////////////////////////////////////
      // 1.3. Get Network Type
      //////////////////////////////////////////////
      USB.print(F("Network mode: "));
      USB.println(_3G.showsNetworkMode(), DEC);
    }
    else
    {
      USB.println(F("1.1. Module NOT connected to network"));
    }
  }
  else
  {
    // Problem with the communication with the 3G module
    USB.println(F("3G module not started"));
    USB.print(F("Error code: "));
    USB.println(answer, DEC);
  }

  ////////////////////////////////////////////////
  // 2.1. FTP open session, send data in filename_old
  ////////////////////////////////////////////////

  answer = _3G.configureFTP(ftp_server, ftp_port, ftp_user, ftp_pass, 1, "I");

  // check answer
  if (answer == 1)
  {
    USB.println(F("2.1. FTP open session OK"));

    previous = millis();

    //////////////////////////////////////////////
    // 2.2. FTP upload
    //////////////////////////////////////////////

    //    snprintf(SERVER_FILE, sizeof(SERVER_FILE), "/incoming/Munch/%s", filename_old );
    //    USB.print(F("Server_file: "));
    //    USB.println(SERVER_FILE);


    answer = _3G.uploadData(filename_old, filename_old);

    if (answer == 1)
    {

      USB.print(F("2.2. Uploading SD file to FTP server done! "));
      USB.print(F("Upload time: "));
      USB.print((millis() - previous) / 1000, DEC);
      USB.println(F(" s"));
    }
    else
    {
      USB.print(F("2.2. Error calling 'ftpUpload' function. Error: "));
      USB.println(answer, DEC);
    }
  }
  else
  {
    USB.print(F( "2.1. FTP connection error: "));
    USB.println(answer, DEC);
  }


  ////////////////////////////////////////////////
  // 3. Send SMS
  ////////////////////////////////////////////////
  if (connection_status)
  {
    USB.println(F("3G module connected to the network..."));

    if (batteryLevel <= 80) // Send SMS varning if battery below 80%
    {
      USB.print(F("2.2. Sending SMS..."));
      answer = _3G.sendSMS(sms_body, phone_number);

      if (answer == 1)
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
  else
  {
    USB.println(F("3G module not connect to the network..."));
  }

  ////////////////////////////////////////////////
  // Wait for satellite signals and get values
  ////////////////////////////////////////////////
  // GPS_status = _3G.startGPS();
  GPS_status = _3G.startGPS(2, "supl.google.com", "7276"); // start GPS in MS-based mode

  if (GPS_status == 1)
  {
    USB.println(F("GPS started"));
  }
  else
  {
    USB.print(F("GPS NOT started, error: "));
    USB.println(GPS_status);
  }

  ////////////////////////////////////////////////
  // Set RTC time
  ////////////////////////////////////////////////
  USB.print(F("Time [Day of week, YY/MM/DD, hh:mm:ss]: "));
  USB.println(RTC.getTime());

  answer = _3G.setTimebyGPS(60, 1);
  if (answer == 1)
  {
    USB.print(F("Time set to [Day of week, YY/MM/DD, hh:mm:ss]: "));
  }
  else
  {
    USB.print(F("Time NOT! changed [Day of week, YY/MM/DD, hh:mm:ss]: "));
  }
  //Show time
  USB.println(RTC.getTime());

  if (GPS_status == 1)
  {
    answer = _3G.getGPSinfo(); // Get GPS information
    if (answer == 1)
    {
      USB.print(F("3. GPS signal received. Time(secs) = "));
      USB.println((millis() - previous) / 1000);

      USB.println(F("Acquired position:"));
      USB.println(F("----------------------------"));

      USB.print(F("Latitude: "));
      USB.println(_3G.latitude);
      USB.print(F("Longitude: "));
      USB.println(_3G.longitude);
      USB.print(F("Date: "));
      USB.println(_3G.date);
      USB.print(F("UTC_time: "));
      USB.println(_3G.UTC_time);
      USB.print(F("Altitude: "));
      USB.println(_3G.altitude);
      USB.print(F("SpeedOG: "));
      USB.println(_3G.speedOG);
      USB.print(F("Course: "));
      USB.println(_3G.course);

      USB.println(F("----------------------------"));

      // get degrees
      gps_latitude  = _3G.convert2Degrees(_3G.latitude);
      gps_longitude = _3G.convert2Degrees(_3G.longitude);

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
      USB.println(answer, DEC);
    }
  }

  // Power off the 3G module
  _3G.OFF();
}






