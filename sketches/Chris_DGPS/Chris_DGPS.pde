/*
  Control of DGPS with Crydom relay
  2018-04-08 (C) john.hulth@geo.uio.no

  Five Green blinkings indicates GOOD battery (long) and/or GPS (short)
  Five RED blinkings indicates BAD battery (long) and/or GPS (short)
  RED light during GPS search
  Green Led indicates relay on and waspmote sleeping

  Define summer and winter dates and turn on/off voltage for battery before deployment!!!

*/
#include <WaspGPS.h>

#define deploy 1 // Set to 1 for deployment and 0 for testing

// define variable for summer and winter season
#define SummerStartMonth 3 // 3
#define SummerStartDate 15 // 15

#define WinterStartMonth 9 // 9
#define WinterStartDate 1 // 1

// define variable for battery voltage
#define TurnOffVoltage 11.5 // 11.5 ~ 50%
#define TurnOnVoltage 12.2 // 12.2 ~ 65-70%

float LeadAcid_V;
char LeadAcid_V_str[10];
bool relay_on;

// define GPS timeout when connecting to satellites
// this time is defined in seconds (180 sec = 3 minutes, 300 sec = 5 min)
#define TIMEOUT 300

// General variables
bool status; // define status variable for GPS connection
char latitude_str[30];
char longitude_str[30];
char filename[13]; // define file name: MUST be 8.3 SHORT FILE NAME
char sample_txt[100]; // text string to append to SD-card

// - - - Start of program - - -
void setup()
{
  USB.print(F("Setup... Check battery and time"));

  // Read battery voltage
  LeadAcid_V = LeadAcidRead();
  USB.print(F("\nBattery voltage: "));
  USB.println(LeadAcid_V);

  USB.print(RTC.getTime());

  if (LeadAcid_V > TurnOffVoltage)
  {
    // Indicate good battery
    Utils.blinkGreenLED(1000, 5);

    // CryDom relay ON
    relay_on = CryDom(1);

    // Update time if waspmote have been reset
    if ( RTC.getEpochTime()  <  RTC.getEpochTime(18, 1, 1, 0, 0, 0))
    {
      // Get basic GPS data and update RTC time
      GPSdata();
    }
  }
  else
  {
    // Indicate bad battery
    Utils.blinkRedLED(1000, 5);

    // CryDom relay OFF
    relay_on = CryDom(0);
  }

  // Create file according to DATE with the following format: [YYMMDD.TXT]
  RTC.getTime();
  SD.ON();
  sprintf(filename, "%02u%02u%02u.TXT", RTC.year, RTC.month, RTC.date);
  if (SD.create(filename))
  {
    USB.print(F("\nfile created:"));
    USB.print(filename);
  }
  else
  {
    USB.print(F("\nfile NOT created (only one file per day)\n"));
  }
  // Print list of filemanes to USB
  SD.ls( LS_R | LS_DATE | LS_SIZE );
  SD.OFF();
}

void loop()
{
  // Set watchdog to reboot after 5 min
  RTC.setWatchdog(5);

  USB.print(F("\nStart loop..."));

  // Read battery voltage
  LeadAcid_V = LeadAcidRead();
  USB.print(F("\nBattery voltage: "));
  USB.print(LeadAcid_V);

  USB.print(F("\nCryDom relay: "));
  USB.print(relay_on);

  RTC.getTime();

  USB.print(F("\nSummer mode: "));
  USB.print(Summer());

  // Logic to control relay on/off
  if (  (Summer() && relay_on && (LeadAcid_V > TurnOffVoltage))
        || (Summer() && (LeadAcid_V > TurnOnVoltage))
        || ((RTC.hour == 12) && (LeadAcid_V > TurnOffVoltage))  )
  {
    relay_on = CryDom(1); // Turn relay ON
  }
  else
  {
    relay_on = CryDom(0); // Turn relay OFF
  }

  // Get basic GPS data, update RTC time and write to SD-card (every hour in summer and at noon during winter)
  if (Summer() || (RTC.hour == 12))
  {
    // Get basic GPS data and update RTC time
    GPSdata();

    // Write data to SD-card
    dtostrf( LeadAcid_V, 1, 2, LeadAcid_V_str); // use dtostrf() to convert from float to string
    Utils.float2String(GPS.convert2Degrees(GPS.latitude, GPS.NS_indicator), latitude_str, 6);
    Utils.float2String(GPS.convert2Degrees(GPS.longitude, GPS.EW_indicator), longitude_str, 6);

    memset(sample_txt, 0, sizeof(sample_txt));
    snprintf(sample_txt, sizeof(sample_txt), "\n20%02u-%02u-%02u %02u:%02u:%02u,%s,%d,%s,%s,%s"
             , RTC.year, RTC.month, RTC.date, RTC.hour, RTC.minute, RTC.second, LeadAcid_V_str, relay_on, longitude_str, latitude_str, GPS.altitude );

    USB.print(sample_txt);

    SD.ON();
    SD.append(filename, sample_txt);
    SD.OFF();
  }

  // Unset watchdog to not reboot during sleep
  RTC.unSetWatchdog();

  if (relay_on) // relay is ON, sleep until next full hour
  {
    Utils.setLED(LED1, LED_ON); // Green LED when relay is on and waspmote sleeps

#if deploy
    PWR.deepSleep("00:00:00:00", RTC_ABSOLUTE, RTC_ALM1_MODE4, ALL_OFF); //Once an hour !!!! use for deployment !!!
#else
    PWR.deepSleep("00:00:00:00", RTC_ABSOLUTE, RTC_ALM1_MODE5, ALL_OFF); //Once a minute
#endif
    Utils.setLED(LED1, LED_OFF); // Green LED OFF
  }
  else // relay is OFF, sleep until 12.00
  {

#if deploy
    PWR.deepSleep("00:12:00:00", RTC_ABSOLUTE, RTC_ALM1_MODE3, ALL_OFF); //Once a day !!!! use for deployment !!!
#else
    PWR.deepSleep("00:00:00:00", RTC_ABSOLUTE, RTC_ALM1_MODE5, ALL_OFF); //Once a minute
#endif
  }

  // After wake up check interruption source
  if ( intFlag & RTC_INT )
  {
    // clear interruption flag
    intFlag &= ~(RTC_INT);
  }
}


// * * * FUNCTIONS * * *

//-------------------------------------------------
// Read Lead acid battery
float LeadAcidRead()
{
  int analog5;
  // float LeadAcid_V;
  float R1 = 10;  // 10k resistor
  float R2 = 2.2; // 2k2 resistor

  pinMode(16, OUTPUT); //ANALOG3 12V_SW ON/OFF SWITCH
  pinMode(17, OUTPUT); //ANALOG4 Lead-Acid meassurement ON/OFF SWITCH
  pinMode(ANALOG5, INPUT);  //ANALOG5 Read data

  digitalWrite(16, HIGH);
  digitalWrite(17, HIGH);
  delay(1000);

  analog5 = analogRead(ANALOG5);

  digitalWrite(16, LOW);
  digitalWrite(17, LOW);

  LeadAcid_V = analog5  * (R1 + R2) / R2 * 3.3 / 1023 ;

  return  LeadAcid_V;
}

//-------------------------------------------------
// Get basic GPS data and update RTC
void GPSdata()
{
  // Set GPS ON
  GPS.ON();
  Utils.setLED(LED0, LED_ON); // Red LED when waiting for a GPS-fix

  // Wait for GPS signal for specific time
  status = GPS.waitForSignal(TIMEOUT);

  Utils.setLED(LED0, LED_OFF); // Red LED off

  // If GPS is connected then get position
  if ( status == true )
  {
    Utils.blinkGreenLED(300, 5);

    // Get GPS position and date/time
    GPS.getPosition();

    // set time in RTC from GPS time (GMT time)
    GPS.setTimeFromGPS();
    USB.print(F("\nRTC time set to: "));
    USB.print(RTC.getTime());

    // Latitude
    USB.print(F("\nLatitude [ddmm.mmmm]: "));
    USB.print(GPS.NS_indicator);
    USB.print(GPS.latitude);

    //Longitude
    USB.print(F("\nLongitude [dddmm.mmmm]: "));
    USB.print(GPS.EW_indicator);
    USB.print(GPS.longitude);

    // Altitude
    USB.print(F("\nAltitude [m]: "));
    USB.print(GPS.altitude);
  }
  else
    // If GPS not connected, blink red led
  {
    USB.println(F("\nGPS TIMEOUT. NOT connected"));
    Utils.blinkRedLED(300, 5);
  }
  // Turn GPS off
  GPS.OFF();
}

//-------------------------------------------------
// CryDom relay on/off

bool CryDom(bool relay_on)
{
  // set DIGITAL3 pin as output
  pinMode(DIGITAL3 , OUTPUT);

  if ( relay_on == true )
  {
    // set DIGITAL3  HIGH
    digitalWrite(DIGITAL3, HIGH);
  }
  else
  {
    // set DIGITAL3  LOW
    digitalWrite(DIGITAL3, LOW);
  }
  // USB.print(F("\nCryDom relay: "));
  // USB.print(relay_on);
  return relay_on;
}

//-------------------------------------------------
// Check season Summer -> true, Winter -> false

bool Summer()
{
  RTC.getTime();

  unsigned long epoch0 = RTC.getEpochTime(RTC.year, 1, 1, 0, 0, 0);
  unsigned long DOY = (RTC.getEpochTime(RTC.year, RTC.month, RTC.date, 0, 0, 0) - epoch0) / 60 / 60 / 24;
  unsigned long DOYsummerStart = (RTC.getEpochTime(RTC.year, SummerStartMonth, SummerStartDate, 0, 0, 0) - epoch0) / 60 / 60 / 24;
  unsigned long DOYwinterStart = (RTC.getEpochTime(RTC.year, WinterStartMonth, WinterStartDate, 0, 0, 0) - epoch0) / 60 / 60 / 24;

  //  USB.print("\n");
  //  USB.println(epoch0);
  //  USB.println(DOY);
  //  USB.println(DOYsummerStart);
  //  USB.println(DOYwinterStart);

  // Check if it is summer
  if ((DOY >= DOYsummerStart) && (DOY < DOYwinterStart))
  {
    return true;
  }
  else
  {
    return false;
  }
}


