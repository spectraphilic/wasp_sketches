/*  
 *  ------ Waspmote Pro Code Example --------
 *
 * John Hulth (uio.no)
 */

//////////////////////////////////////////
// Included Waspmote Libraries
//////////////////////////////////////////
#include <WaspXBeeDM.h>
#include <WaspFrame.h>
#include <WaspGPS.h>
#include <WaspSensorCities.h>
#include <WaspStackEEPROM.h>

//////////////////////////////////////////
// User selectable variables
//////////////////////////////////////////

// Set power mode, 1 if using hibernate mode, 0 to use deep sleep mode
#define HIBERNATE_MODE 0

// Set sampling interval for measurements
char sampleRate[] = "00:00:00:05"; // "dd:hh:mm:ss"

// Set interval to log measurements
char logRate[] = "00:00:00:10"; // "dd:hh:mm:ss"

// Set interval to transmitt data to base station and get GPS position and uppdate date/time from GPS
char sendRate[]  = "00:00:01:00"; // "dd:hh:mm:ss"

char RX_ADDRESS[] = "0013A20040db6048";

#define GPS_TIMEOUT 120 // "GPS_TIMEOUT 240" (GPS is not used if set to 0 'zero')


//////////////////////////////////////////
// Global variables declaration
//////////////////////////////////////////

// define variable
uint8_t tx_error; // Xbee send error
uint8_t ephemState; // Xbee send error
unsigned long id; // WaspMote serial ID

uint32_t sampleR;
uint32_t logR;
uint32_t sendR;

uint16_t counter_sample;
uint16_t counter_log;
uint16_t counter_send;

timestamp_t alarmTime;    // offset time for interuption alarm
uint32_t alarmTimeSecond;
char alarmTimeString[12];
uint32_t sod;
uint32_t dt;
uint32_t soda;
char TimeString[20]; // Home made string for timestamp

float x_acc; 
float y_acc; 
float z_acc;
float angle_x; 
float angle_y; 
float angle_z;
float tilt_x;
float tilt_y;

float tca; // Air temperature
float us; // Ultra sonic distance


//////////////////////////////////////////
// Module initialization after startup or reset
//////////////////////////////////////////
void setup() 
{
#if HIBERNATE_MODE
  PWR.ifHibernate();
#endif

  // Checks if we come from a normal reset or an hibernate reset
  // Don't run the setup part of the program after a hibernate wake up
  if (!(intFlag & HIB_INT))
  {
    // Convert time rate string ("dd:hh:mm:ss") to integer and check time rate inputs
    uint8_t sampleRate_hour = atoi(&sampleRate[3]);
    uint8_t sampleRate_minute = atoi(&sampleRate[6]); // Put this directly into sampleR etc...
    uint8_t sampleRate_second = atoi(&sampleRate[9]);
    uint8_t logRate_hour = atoi(&logRate[3]);
    uint8_t logRate_minute = atoi(&logRate[6]);
    uint8_t logRate_second = atoi(&logRate[9]);
    uint8_t sendRate_hour = atoi(&sendRate[3]);
    uint8_t sendRate_minute = atoi(&sendRate[6]);
    uint8_t sendRate_second = atoi(&sendRate[9]);

    // Calculate time rates in secounds
    sampleR = (sampleRate_hour*SECS_PER_HOUR + sampleRate_minute*60 + sampleRate_second);  // sampleRate in seconds
    logR = (logRate_hour*SECS_PER_HOUR + logRate_minute*SECS_PER_MIN + logRate_second);    // logRate in seconds
    sendR = (sendRate_hour*SECS_PER_HOUR + sendRate_minute*SECS_PER_MIN + sendRate_second);// sendRate in seconds

    EEPROMupdateLong(1024, sampleR); // Update sampleR to eeprom adress 1024-1027 
    EEPROMupdateLong(1028, logR);    // Update logR to eeprom adress 1028-1031 
    EEPROMupdateLong(1032, sendR);   // Update sendR to eeprom adress 1032-1035 

    // Check time rate inputs valus
    if (((logR%sampleR) || (sendR%logR) || SECS_PER_DAY%sampleR) != 0)
    {
Utils.setLED(LED0, LED_ON); // Set red led on to indicate error
      USB.println("ERROR: logRate, sampleRate and sendRate must be modulus with each other and 'SECS_PER_DAY'");
      USB.println([sampleR,sendR,logR,SECS_PER_DAY],4 ); //Debug info!
    }

    // Initialize counters
    counter_sample = 0;
    counter_log = 0;
    counter_send = 0;

#if HIBERNATE_MODE
    EEPROMupdateLong(1036, counter_sample); // Reset counter_sample to eeprom adress 1036-1039 
    EEPROMupdateLong(1040, counter_log);    // Reset counter_log to eeprom adress 1040-1043 
    EEPROMupdateLong(1044, counter_send);   // Reset counter_send to eeprom adress 1044-1047 
#endif

    // Set time at startup
    if (GPSupdate() == 0)
    {
      // No GPS connection or GPS not in use, dummy time is set in RTC
      RTC.ON();
      RTC.setTime("13:01:01:07:23:58:21");
      Utils.blinkGreenLED(1000); // 1 sec green blink to indicate time updated
    }
    USB.println(RTC.getTimestamp()); //Debug info!

    // Go to sleep after setup
    goToSleep(); // Function to set RTC alarm interuption
  }
}



//////////////////////////////////////////
// Program starts - Waspmote is awake!
//////////////////////////////////////////
void loop() 
{
#if HIBERNATE_MODE
  sampleR = EEPROMreadLong(1024); // Read sampleR from eeprom adress 1024-1027 
  logR = EEPROMreadLong(1028);    // Read logR from eeprom adress 1028-1031 
  sendR = EEPROMreadLong(1032);   // Read sendR from eeprom adress 1032-1035 

  counter_sample = EEPROMreadLong(1036); // Read counter_sample to eeprom adress 1036-1039 
  counter_log = EEPROMreadLong(1040);    // Read counter_log to eeprom adress 1040-1043 
  counter_send = EEPROMreadLong(1044);   // Read counter_send to eeprom adress 1044-1047 

  soda = EEPROMreadLong(1048); // Read 'soda' from eeprom adress 1048-1051 after wakeup
#endif


  /*  
   *  ------ Take measurement sample here -------- 
   1) Take samples (battery, temp etc... 
   2) Calculate Avg, Min etc...  Avg=(X*(n-1) + x)/n ???
   3) Store data in EEPROM
   */
  counter_sample++; // 
  Utils.blinkGreenLED(); // 200 ms blink

  /* // Home made time string because of ~2 sec delay in hibernate function
   RTC.ON();
   RTC.getAlarm1();
   RTC.getTimestamp();
   snprintf(TimeString, sizeof(TimeString), "20%02u-%02u-%02u %02u:%02u:%02u",\
   RTC.year, RTC.month, RTC.day_alarm1, RTC.hour_alarm1, RTC.minute_alarm1, RTC.second_alarm1);
   RTC.OFF();
   */

  getTilt(); // Function to calculate tilt out of accelerometer (tilt_x, tilt_y)


  // Read sensors connected to the sensor board
  SensorCities.ON(); //Turn on board
  SensorCities.setSensorMode(SENS_ON, SENS_CITIES_ULTRASOUND_5V); //Turn on sensor
  SensorCities.setSensorMode(SENS_ON, SENS_CITIES_TEMPERATURE); //Turn on sensor
  delay(3000);

  tca = SensorCities.readValue(SENS_CITIES_TEMPERATURE); 
  us = SensorCities.readValue(SENS_CITIES_ULTRASOUND_5V, SENS_US_WRA1); 

  SensorCities.OFF(); //Turn off board


  USB.printf("\n--------------Sample#: %u-------------\n",counter_sample); // Print 'Sample number', resets after logging data
  USB.println(RTC.getTimestamp());                        // Print Timestamp
  USB.println(TimeString);                                // Print timeString (Obs! home made time string, 2 sec delay in hibernate function)
  USB.printf("Free Memory: %u bytes\n",freeMemory());     // Print Free Memory
  USB.print(F("Battery (%): "));                          // Print Battery level string
  USB.println(PWR.getBatteryLevel(),DEC);                 // Print Battery level float
  USB.print(F("Battery (V): "));                          // Print Battery volt string
  USB.println(PWR.getBatteryVolts());                     // Print Battery volt float
  USB.print(F(" - tilt_x (deg): "));                      // Print Tilt x string
  USB.println(tilt_x);                                    // Print Tilt x float
  USB.print(F(" | tilt_y (deg): "));                      // Print Tilt y string
  USB.println(tilt_y);                                    // Print Tilt y float
  USB.print(F("tca (C): "));                              // Print tca string
  USB.println(tca);                                       // Print tca float  
  USB.print(F("us (cm):  "));                             // Print us string
  USB.println(us);                                        // Print us float


  //delay(515+counter_sample);







  /*  
   *  ------ If Time is modulus with logRate, Save data here -------- 
   1) Check if time is modulus with logRate
   2) Take 'slow' measurements (timestamp, snowdepth etc... 
   3) Read samples from EEPROM
   4) Save data to SD
   5) Push frames to stack for sending
   */
  if ((soda%logR) == 0)
  {
    counter_log++;

    USB.println(F("LOGGING!!!!!!"));
    USB.println(soda);

    USB.printf("Log#: %u\n",counter_log);   // Print 'Log number'

    GPSupdate(); // Updates the location, time and date from the GPS 









    counter_sample = 0; // Reset sample counter after logging
  }
  // !!!
  /*  
   *  ------ Send data here if using cyclic sleep with XBEE -------- 
   *               DM 09: Set cyclic sleep mode
   1) Check if time is modulus with sendRate
   2) Check status, errors, low battery etc... 
   3) Changes power setings if needed
   4) Turn GPS on (GPS 05: XBeeDM send)
   5) Pop frames from stackEEPROM and send data
   6) Update position and time from GPS
   */
  if ((soda%sendR) == 0)
  {
    counter_send++;

    USB.println(F("SENDING DATA!!!!!!"));
    USB.println(soda);

    // create new ASCII frame
    frame.createFrame(ASCII,"");  
    frame.addSensor(SENSOR_TST, RTC.getTimestamp());
    frame.addSensor(SENSOR_BAT, PWR.getBatteryLevel()); 
    frame.addSensor(SENSOR_TCA, tca); 
    frame.addSensor(SENSOR_US,  us); 

    /* // send XBee packet
     tx_error = xbeeDM.send( RX_ADDRESS, frame.buffer, frame.length );   
     // check TX flag
     if( tx_error == 0 )
     {
     frame.showFrame();
     }
     else 
     {
     USB.println(F("send error"));
     USB.println(tx_error);
     }
     */

    frame.showFrame();




  }







  //  // Check Interruption register for XBEE interuption
  //  if (intFlag & XBEE_INT)
  //  {
  //    intFlag &= ~(XBEE_INT); // Clear flag
  //
  //  }





  // Update counters
#if HIBERNATE_MODE
  EEPROMupdateLong(1036, counter_sample); // Update counter_sample to eeprom adress 1036-1039 
  EEPROMupdateLong(1040, counter_log);    // Update counter_log to eeprom adress 1040-1043 
  EEPROMupdateLong(1044, counter_send);   // Update counter_send to eeprom adress 1044-1047
#endif

  // Waspmote goes to sleep 
  goToSleep(); 
} // End of program - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

//////////////////////////////////////////
// goToSleep: Function find the sleep time to next wake up
// and get the waspmote to sleep (Hibernate or deep sleep)
//////////////////////////////////////////

// goToSleep : Function to set RTC alarm interuption and Set WaspMote to sleep
uint32_t goToSleep(void)
{
  RTC.ON();
  RTC.getTime();

  // Find time for the next alarm
  sampleR = EEPROMreadLong(1024); // Read sampleR from eeprom adress 1024-1027 
  sod = RTC.hour * SECS_PER_HOUR + RTC.minute * SECS_PER_MIN + RTC.second; // second of day
  dt = sod % sampleR; // seconds since 'previus' alarm time
  soda = sod + sampleR - dt; // second of day for next alarm time

#if HIBERNATE_MODE 
  soda = soda-2; // Minus 2 sec to compensate for the delay in the hibernate function
#endif

  if (soda > SECS_PER_DAY)
  {
    soda = SECS_PER_DAY;
  } 
  alarmTimeSecond = soda - sod; // New alarm time offset in seconds

  // New alarm time offset in timestamp_t strukture
  RTC.breakTimeOffset(alarmTimeSecond, &alarmTime); 

  // New alarm time offset in string format "dd:hh:mm:ss"
  snprintf(alarmTimeString, sizeof(alarmTimeString), "%02u:%02u:%02u:%02u", alarmTime.date, alarmTime.hour, alarmTime.minute, alarmTime.second );

  //Debug info!
  USB.printf("sod: %lu\n",sod);  
  USB.printf("sampleR: %lu\n",sampleR);  
  USB.printf("dt: %lu\n",dt);  
  USB.printf("soda: %lu\n",soda);
  USB.printf("Alarm time offset: %s\n",alarmTimeString);

  if (intFlag & HIB_INT)   // Check Interruption register for hibernate alarm
  {
    intFlag &= ~(HIB_INT); // Clear flag
  }
  if (intFlag & RTC_INT)   // Check Interruption register for RTC alarm
  {
    intFlag &= ~(RTC_INT); // Clear flag
  }

  // Set Waspmote to sleep (hibernate or deep sleep)
#if HIBERNATE_MODE // Set waspmote to hibernate
  soda = soda+2; // Addd 2 sec to get the correct soda in hibernate mode

  USB.println(millis()); //Debug info!!!!!!!!!!!!!!!!!!!!!!!!!

  EEPROMupdateLong(1048, soda); // Update 'soda' to eeprom adress 1048-1051 
  USB.println(F("WaspMote goes into Hibernate...")); //Debug info!
  PWR.hibernate(alarmTimeString, RTC_OFFSET,RTC_ALM1_MODE2);
  return soda;

#else // Set Waspmote to deep sleep

  USB.println(millis()); //Debug info!!!!!!!!!!!!!!!!!!!!!!!!!


  USB.println(F("WaspMote goes into Deep sleep...")); //Debug info!
  PWR.deepSleep(alarmTimeString,RTC_OFFSET,RTC_ALM1_MODE2,ALL_OFF);
  return soda;

#endif
}

//////////////////////////////////////////
// Updates the location, time and date from the GPS 
// Returns: 0 not updated, 1 updated, 2 updated and ephems saved  
//////////////////////////////////////////
uint8_t GPSupdate(void)
{
#if (GPS_TIMEOUT == 0) // If GPS_TIMEOUT = 0, GPS is not in use
  {
    return 0;
  }
#else
  {
    GPS.ON();
    SD.ON();

    ephemState = GPS.loadEphems();
    if (ephemState == 1)
    {
      USB.println(F("Ephemeris loaded successfully"));
    }
    else 
    {
      USB.print(F("Ephemeris not loaded: "));
      USB.println(ephemState, DEC);
    }
    GPS.ON();
    SD.ON();
    if (GPS.waitForSignal(GPS_TIMEOUT))
    {
      GPS.setTimeFromGPS(); // RTC.ON() is in this function
      GPS.getPosition();
      USB.println("GPS ok");

      ephemState = GPS.saveEphems();
      if (ephemState == 1)
      {
        USB.println(F("Ephemeris stored successfully"));
        GPS.OFF();
        return 2;
      }
      else 
      {
        USB.print(F("Ephemeris not saved: "));
        USB.println(ephemState, DEC);
      }
      GPS.OFF();
      return 1;
    }
    GPS.OFF();
    return 0;
  }
#endif
}

//////////////////////////////////////////
// getTilt : Function to calculate tilt 
// out of accelerometer readings(tilt_x, tilt_y)
//////////////////////////////////////////
float getTilt(void)
{
  ACC.ON(FS_2G);
  delay(100); // Needed for the accelerometers high-pass filter

  x_acc = ACC.getX();
  y_acc = ACC.getY();
  z_acc = ACC.getZ();

  ACC.OFF();

  angle_x = (atan2(x_acc,sqrt((y_acc*y_acc)+(z_acc*z_acc))))*180/PI;
  angle_y = (atan2(y_acc,sqrt((x_acc*x_acc)+(z_acc*z_acc))))*180/PI;
  angle_z = (atan2(z_acc,sqrt((x_acc*x_acc)+(y_acc*y_acc))))*180/PI;

  if ((abs(z_acc)>abs(x_acc)) && (abs(z_acc)>abs(y_acc)) &&(z_acc>=0))
  {
    tilt_x = angle_x;
    tilt_y = angle_y;
  }
  else if ((abs(z_acc)>abs(x_acc)) && (abs(z_acc)>abs(y_acc)) &&(z_acc<0))
  {
    tilt_x = angle_y;
    tilt_y = angle_x;
  }
  else if ((abs(x_acc)>abs(y_acc)) && (abs(x_acc)>abs(z_acc)) &&(x_acc>=0))
  {
    tilt_x = angle_y;
    tilt_y = angle_z;
  }
  else if ((abs(x_acc)>abs(y_acc)) && (abs(x_acc)>abs(z_acc)) &&(x_acc<0))
  {
    tilt_x = angle_z;
    tilt_y = angle_y;
  }
  else if ((abs(y_acc)>abs(x_acc)) && (abs(y_acc)>abs(z_acc)) &&(y_acc>=0))
  {
    tilt_x = angle_z;
    tilt_y = angle_x;
  }
  else if ((abs(y_acc)>abs(x_acc)) && (abs(y_acc)>abs(z_acc)) &&(y_acc<0))
  {
    tilt_x = angle_x;
    tilt_y = angle_z;
  }
  else
  {
    tilt_x = 99.9;
    tilt_y = 99.9;
  }

  /* //Debug info!
   USB.print(F("Acceleration X: "));
   USB.println(x_acc,DEC);
   USB.print(F("Acceleration Y: "));
   USB.println(y_acc,DEC);
   USB.print(F("Acceleration Z: "));
   USB.println(z_acc,DEC);
   */

  ACC.OFF();
  return (tilt_x, tilt_y);
}



//////////////////////////////////////////
// EEPROMwriteLong: Function to write a 4 byte long
// to the eeprom at the specified address to address + 3.
//////////////////////////////////////////
void EEPROMwriteLong(int address, long value)
{
  //Decomposition from a long to 4 bytes by using bitshift.
  //One = Most significant -> Four = Least significant byte
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);

  //Write the 4 bytes into the eeprom memory.
  Utils.writeEEPROM(address, four);
  Utils.writeEEPROM(address + 1, three);
  Utils.writeEEPROM(address + 2, two);
  Utils.writeEEPROM(address + 3, one);
}

//////////////////////////////////////////
// EEPROMreadLong: Function to write a 4 byte long 
// to the eeprom at the specified address to address + 3.
//////////////////////////////////////////
long EEPROMreadLong(int address)
{
  //Read the 4 bytes from the eeprom memory.
  long four = Utils.readEEPROM(address);
  long three = Utils.readEEPROM(address + 1);
  long two = Utils.readEEPROM(address + 2);
  long one = Utils.readEEPROM(address + 3);

  //Return the recomposed long by using bitshift.
  return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
}

//////////////////////////////////////////
// EEPROMupdateLong: Function to update a 4 byte long 
// to the eeprom at the specified address to address + 3.
//////////////////////////////////////////
bool EEPROMupdateLong(int address, long value)
{
  long _value = (EEPROMreadLong(address));
  if (_value == value)
  {
    return 0;
  }
  else 
  {
    EEPROMwriteLong(address, value);
    return 1;
  }
}





























































































