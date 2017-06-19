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
// Different values for different battery levels.
struct Sampling {
  const uint8_t offset;
  const char* offset_str;
};

Sampling samplings[3] = {
  { 12, "00:00:12:00" }, // <= 30
  {  4, "00:00:04:00" }, // <= 55
  {  2, "00:00:02:00" }, //  > 55
};

Sampling* sampling = NULL;


// 3. Global variables declaration
bool error;
uint8_t alarmMinutes;
char alarmTime[] = "00:00:00:00";
int pendingPulses;
int minutes;
int hours;
int randomNumber;
uint8_t batteryLevel;

const char* targetUnsentFile;


Sampling getSampling() {
  if (batteryLevel <= 30) {
    sampling = &samplings[0];
  } else if (batteryLevel <= 55) {
    sampling = &samplings[1];
  } else {
    sampling = &samplings[2];
  }
}


void setup()
{
  // Flags to turn USB print, OTA programming ON or OFF
  UIO.USB_output = 1;   // turn print to USB ON/OFF

  // Turn on the sensor board
  SensorAgrv20.ON();
  delay(100);

  UIO.start_RTC_SD_USB();
  USB.println("Wasp started, Agr board ON");

  UIO.interactive();

  xbeeDM.ON();
  delay(50);

  // Function to initialize SD card
  error = UIO.initSD();
  UIO.println(UIO.archive_file);
  delay(100);

  UIO.logActivity("Waspmote starting");
  targetUnsentFile = UIO.unsent_fileA;

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
  batteryLevel = PWR.getBatteryLevel();
  getSampling();

  RTC.getTime();
  alarmMinutes = (RTC.minute / sampling->offset) * sampling->offset + sampling->offset;
  if (alarmMinutes >= 60)
    alarmMinutes = 0;
  sprintf(alarmTime, "00:00:%02d:00", alarmMinutes);

  UIO.stop_RTC_SD_USB();

  SensorAgrv20.sleepAgr(alarmTime, RTC_ABSOLUTE, RTC_ALM1_MODE4, ALL_OFF);
}


void loop()
{
  UIO.start_RTC_SD_USB();

  // Battery level
  batteryLevel = PWR.getBatteryLevel();
  UIO.logActivity("Loop, battery level = %d", batteryLevel);

  // Check RTC interruption
  if (intFlag & RTC_INT)
  {
    UIO.logActivity("+ RTC interruption +");
    Utils.blinkGreenLED(); // blink green once every minute to show it is alive

    // Battery too low, do nothing
    if (batteryLevel <= 30) {
      goto sleep;
    }

    // Measure sensors
    UIO.measureAgriSensorsBasicSet();
    error = UIO.frame2archive(UIO.tmp_file, false);

    RTC.getTime();
    minutes = RTC.minute;
    hours = RTC.hour;

    //-----------------------------------------
    if ((batteryLevel > 65) && (batteryLevel <= 75)) {
      // Attempt sending data every 3 hours, if battery <= 75%
      if (hours % 3 == 0) {
        UIO.frame2Meshlium(UIO.tmp_file, targetUnsentFile);
      }
    }

    //-----------------------------------------
    if (batteryLevel > 75) {
      // Attempt sending data every hour, if battery > 75%
      if (minutes == 0) {
        UIO.frame2Meshlium(UIO.tmp_file, targetUnsentFile);
      }

      // GPS time synchronyzation once a day, at 13:00
      if (hours == 13 && minutes == 0) {
        UIO.receiveGPSsyncTime();

        // Once a day try to send all data in current unsent file.
        if (targetUnsentFile == UIO.unsent_fileA) {
          UIO.frame2Meshlium(targetUnsentFile, UIO.unsent_fileB);
          UIO.delFile(UIO.unsent_fileA);
          UIO.createFile(UIO.unsent_fileA);
          targetUnsentFile = UIO.unsent_fileB;
        }
        if (targetUnsentFile == UIO.unsent_fileB) {
          UIO.frame2Meshlium(targetUnsentFile, UIO.unsent_fileA);
          UIO.delFile(UIO.unsent_fileB);
          UIO.createFile(UIO.unsent_fileB);
          targetUnsentFile = UIO.unsent_fileA;
        }
      }
    }
  }

sleep:
  UIO.stop_RTC_SD_USB();

  // Clear interruption flag & pin
  clearIntFlag();
  PWR.clearInterruptionPin();

  // Set whole agri board and waspmote to sleep, until next alarm.
  getSampling();
  SensorAgrv20.sleepAgr(sampling->offset_str, RTC_OFFSET, RTC_ALM1_MODE4, ALL_OFF);
}
