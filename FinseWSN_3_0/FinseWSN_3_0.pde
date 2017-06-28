/*
 SCRIPT for Finse network, to synchronize DM network, and read basic set of sensors
 April 2017, Simon Filhol

 Script description:

 */

// 1. Include Libraries
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
unsigned long time;

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
  time = millis();

  // Initialize variables, from EEPROM (USB print, OTA programming, ..)
  UIO.initVars();

  // Log
  UIO.start_RTC_SD_USB();
  batteryLevel = PWR.getBatteryLevel();
  UIO.logActivity(F("INFO <<< Booting. Agr board ON. Battery level is %d"), batteryLevel);

  // Interactive mode
  UIO.interactive();

  // Create files in SD
  error = UIO.initSD();
  targetUnsentFile = UIO.unsent_fileA;

  // set random seed
  //srandom(42);

  //UIO.readOwnMAC();
  UIO.logActivity(F("INFO >>> Boot done in %d ms"), UIO.millisDiff(time, millis()));

  // Calculate first alarm (requires batteryLevel)
  getSampling();

  RTC.getTime();
  alarmMinutes = (RTC.minute / sampling->offset) * sampling->offset + sampling->offset;
  if (alarmMinutes >= 60)
    alarmMinutes = 0;
  sprintf(alarmTime, "00:00:%02d:00", alarmMinutes);

  // Go to sleep
  UIO.stop_RTC_SD_USB();
  PWR.deepSleep(alarmTime, RTC_ABSOLUTE, RTC_ALM1_MODE4, ALL_OFF);
}


void loop()
{
  time = millis();
  UIO.start_RTC_SD_USB();

  // Update RTC time at least once. Kepp minute and hour for later.
  RTC.getTime();
  minutes = RTC.minute;
  hours = RTC.hour;

  // Check RTC interruption
  if (intFlag & RTC_INT)
  {
    // Battery level, do nothing if too low
    batteryLevel = PWR.getBatteryLevel();
    if (batteryLevel <= 30) {
      UIO.logActivity("DEBUG RTC interruption, low battery = %d", batteryLevel);
      goto sleep;
    }

    UIO.logActivity("INFO RTC interruption, battery level = %d", batteryLevel);
    //Utils.blinkGreenLED(); // blink green once every minute to show it is alive

    // Warm sensors
    UIO.warmSensors();

    // Send frames once an hour at most
    bool sendFrames = (
      (batteryLevel > 75 && minutes == 0) ||                 // Once an hour
      (batteryLevel > 65 && minutes == 0 && hours % 3 == 0)  // Once every 3 hours
    );

    // Sync time once a day
    bool syncTime = (
      (batteryLevel > 75 && hours == 13 && minutes == 0)
    );

    unsigned long start = millis();
    unsigned long diff;
    while (true)
    {
      diff = UIO.millisDiff(start, millis());

      if (sendFrames)
      {
        sendFrames = false; // Flag not to call this twice
        UIO.frame2Meshlium(UIO.tmp_file, targetUnsentFile);
      }

      if (syncTime)
      {
        syncTime = false; // Flag not to call this twice

        // GPS time synchronyzation once a day, at 13:00
        UIO.receiveGPSsyncTime();

        // FIXME This is not robust to reboots, as the state (of which is the
        // unsent file) is kept in memory. State must be persistent to be robust.
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

      // Last (exit) action!!
      // Read sensors once they are warm, 10s after
      if (diff > 10000)
      {
        // XXX Sensors read this loop will be sent next time
        UIO.readSensors();
        break; // Last action!! go to sleep
      }

    }
  }
  else
  {
    UIO.logActivity(F("WARN Unexpected interruption %d"), intFlag);
  }

sleep:
  time = UIO.millisDiff(time, millis());
  UIO.logActivity("INFO Loop done in %lu ms.", time);
  UIO.stop_RTC_SD_USB();

  // Clear interruption flag & pin
  clearIntFlag();
  PWR.clearInterruptionPin();

  // Set whole agri board and waspmote to sleep, until next alarm.
  getSampling();
  SensorAgrv20.sleepAgr(sampling->offset_str, RTC_OFFSET, RTC_ALM1_MODE4, ALL_OFF);
}
