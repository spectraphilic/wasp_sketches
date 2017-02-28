/*  
 SCRIPT for Finse network, to synchronize DM network, and read basic set of sensors
 14 September 2016, Simon Filhol, John Hulth
 
 Script description:
 - sample sensors every 10 minutes
 - OTA once a week (Tuesday 12pm UTC = 2pm Oslo) if battery and connection to Meshlium exist. OTA accessible for 4 min
 - send data to Meshlium every hour if battery > 60% otherwise only every 6hours
 - Try sendind nusent frames only once a day. If not able to send data for the last 20000 frames, move frames to permanent storage on SD
 - Update time from GPS unit once a day
 - If battery power is less than 30%, waspmote only sample from sensors, save data to SD and nothing else.
 
 */

// Put your libraries here (#include ...)
#include <WaspXBeeDM.h>
#include <WaspUIO.h>
#include <WaspSensorAgr_v20.h>
#include <WaspFrame.h>

// Define local variables:
char message[40];
int pendingPulses;
int minutes;
int hours;
int randomNumber;
int batteryLevel;

// define the sampling period in minute MUST BE between 0 and 29 minute
int SamplingPeriod = 10;



void setup()
{
  // Flags to turn USB print, OTA programming ON or OFF
  UIO.USB_output = 1;   // turn print to USB ON/OFF
  
  USB.ON();
  RTC.ON();
  xbeeDM.ON();
  delay(50);

  // Function to initialize SD card
  UIO.initSD();
  UIO.logActivity("Waspmote starting");

  // Function to initialize
  UIO.initNet('Finse');
  UIO.logActivity("SD,XbeeDM initialized");

  // Turn on the sensor board
  SensorAgrv20.ON();
  delay(5000);

  // set random seed
  srandom(42);

  UIO.readBatteryLevel();
  UIO.logActivity("Waspmote set and ready");

  xbeeDM.OFF();
  USB.OFF();
  RTC.OFF();
}


void loop()
{

  // set whole agri board and waspmote to sleep. Wake up every minute to
  SensorAgrv20.sleepAgr("00:00:00:00", RTC_ABSOLUTE, RTC_ALM1_MODE5, ALL_OFF);

  // extract RTC time
  RTC.ON();
  RTC.getTime();
  minutes = RTC.minute;
  hours = RTC.hour;
  RTC.OFF();

  //Check RTC interruption
  if(intFlag & RTC_INT)
  {
    UIO.start_RTC_SD_USB();
    UIO.logActivity("+ RTC interruption +");
    Utils.blinkGreenLED(); // blink green once every minute to show it is alive

    // Sample every sampling period minutes
    if ((minutes%SamplingPeriod)==0)
    { 
      // Measure sensors
      sprintf(message, "+ %d min Sampling +", SamplingPeriod);
      UIO.logActivity(message);
      batteryLevel = PWR.getBatteryLevel()

      //-----------------------------------------
      if((batteryLevel > 30) && (batteryLevel =< 55)){
        // Measure sensors every 2 sampling period
        if(minutes%(SamplingPeriod*2) == 0){
           UIO.measureAgriSensorsBasicSet();
         }
      }

      //-----------------------------------------
      if((batteryLevel > 55) && (batteryLevel =< 65)){
        // Measure sensors every sampling period
        UIO.measureAgriSensorsBasicSet();
      }

      //-----------------------------------------
      if((batteryLevel > 65) && (batteryLevel =< 75)){
        // Measure sensors every sampling period
        UIO.measureAgriSensorsBasicSet();

        // Attempt sending data every 3 hours
        if(hours%3 == 0){
          // send data
          UIO.Frame2Meshlium();
        }
      }

      //-----------------------------------------
      if(batteryLevel > 75){
        // Measure sensors every sampling period
        UIO.measureAgriSensorsBasicSet();
        if(minutes == 0){
          //send data
          UIO.Frame2Meshlium();
        }
        if(hours == 13){
          // GPS time synchronyzation
          UIO.receiveGPSsyncTime();
        }

      }
    }
 
  // Clear flag
  intFlag &= ~(RTC_INT); 
  UIO.stop_RTC_SD_USB();

  clearIntFlag(); 
  PWR.clearInterruptionPin();
  }
}


