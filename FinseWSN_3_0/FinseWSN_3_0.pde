/*
 SCRIPT for Finse network, to synchronize DM network, and read basic set of sensors
 April 2017, Simon Filhol

 Script description:

 */

// 1. Include Libraries
#include <WaspXBeeDM.h>
#include <WaspUIO.h>
#include <WaspSensorAgr_v20.h>
#include <WaspFrame.h>

// 2. Definitions

// Sampling period in minutes, keep these two definitions in sync. The value
// must be a factor of 60 to get equally spaced samples.
// Possible values: 1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30
#define SAMPLING_PERIOD 2
#define SAMPLING_PERIOD_STR "00:00:02:00"

// 3. Global variables declaration
uint8_t error;
uint8_t alarmMinutes;
char alarmTime[] = "00:00:00:00";
char message[100];
int pendingPulses;
int minutes;
int hours;
int randomNumber;
int batteryLevel;

char* targetUnsentFile;
String archive_file = " ";

void setup()
{
  // Flags to turn USB print, OTA programming ON or OFF
  UIO.USB_output = 1;   // turn print to USB ON/OFF

  // Turn on the sensor board
  SensorAgrv20.ON();
  delay(100);

  UIO.start_RTC_SD_USB();
  USB.println("Wasp started, Agr board ON");

  UIO.setTimeFromUser();

  xbeeDM.ON();
  delay(50);

  // Function to initialize SD card
  archive_file = UIO.initSD();
  UIO.println(archive_file);
  delay(100);

  UIO.logActivity("Waspmote starting");
  targetUnsentFile = (char*)UIO.unsent_fileA;

  // Function to initialize
  UIO.initNet(NETWORK_BROADCAST);
  UIO.logActivity("SD,XbeeDM initialized");


  // set random seed
  //srandom(42);

  //UIO.readOwnMAC();
  UIO.readBatteryLevel();
  UIO.logActivity("Waspmote set and ready");

  xbeeDM.OFF();

  // Calculate first alarm
  RTC.getTime();
  alarmMinutes = (RTC.minute / SAMPLING_PERIOD) * SAMPLING_PERIOD + SAMPLING_PERIOD;
  if (alarmMinutes >= 60)
    alarmMinutes = 0;
  sprintf(alarmTime, "00:00:%02d:00", alarmMinutes);

  USB.OFF();
  RTC.OFF();

  SensorAgrv20.sleepAgr("00:00:00:00", RTC_ABSOLUTE, RTC_ALM1_MODE5, ALL_OFF);
}


void loop()
{
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

    // Measure sensors
    sprintf(message, "+ %d min Sampling +", SAMPLING_PERIOD);
    UIO.logActivity(message);
    batteryLevel = PWR.getBatteryLevel();

    //-----------------------------------------
    if((batteryLevel > 30) && (batteryLevel <= 55)){
      // Measure sensors every 2 sampling period
      if(minutes%(SAMPLING_PERIOD*2) == 0){
         UIO.measureAgriSensorsBasicSet();
         archive_file = UIO.frame2archive(UIO.tmp_file, archive_file, false);
       }
    }

    //-----------------------------------------
    if((batteryLevel > 55) && (batteryLevel <= 65)){
      // Measure sensors every sampling period
      UIO.measureAgriSensorsBasicSet();
      archive_file = UIO.frame2archive(UIO.tmp_file, archive_file, false);
    }

    //-----------------------------------------
    if((batteryLevel > 65) && (batteryLevel <= 75)){
      // Measure sensors every sampling period
      UIO.measureAgriSensorsBasicSet();
      archive_file = UIO.frame2archive(UIO.tmp_file, archive_file, false);

      // Attempt sending data every 3 hours
      if(hours%3 == 0){
        // send data
        UIO.frame2Meshlium(UIO.tmp_file, targetUnsentFile);
      }
    }

    //-----------------------------------------
    if(batteryLevel > 75){
      // Measure sensors every sampling period
      UIO.measureAgriSensorsBasicSet();
      archive_file = UIO.frame2archive(UIO.tmp_file, archive_file, false);

      if(minutes == 0){
        //send data
        UIO.frame2Meshlium(UIO.tmp_file, targetUnsentFile);
      }
      if(hours == 13){
        // GPS time synchronyzation
        UIO.receiveGPSsyncTime();

        // Once a day try to send all data in current unsent file.
        if (targetUnsentFile == UIO.unsent_fileA) {
          UIO.frame2Meshlium(targetUnsentFile, UIO.unsent_fileB);
          UIO.delFile(UIO.unsent_fileA);
          UIO.createFile(UIO.unsent_fileA);
          targetUnsentFile = (char*)UIO.unsent_fileB;
        }
        if (targetUnsentFile == UIO.unsent_fileB) {
          UIO.frame2Meshlium(targetUnsentFile, UIO.unsent_fileA);
          UIO.delFile(UIO.unsent_fileB);
          UIO.createFile(UIO.unsent_fileB);
          targetUnsentFile = (char*)UIO.unsent_fileA;
        }
      }
    }

    // Clear flag
    intFlag &= ~(RTC_INT);
    UIO.stop_RTC_SD_USB();

    clearIntFlag();
    PWR.clearInterruptionPin();
  }

  // Set whole agri board and waspmote to sleep, until next alarm.
  SensorAgrv20.sleepAgr(SAMPLING_PERIOD_STR, RTC_OFFSET, RTC_ALM1_MODE4, ALL_OFF);
}
