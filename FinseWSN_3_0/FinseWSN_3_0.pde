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

// 3. Global variables declaration
uint8_t error;
const char* alarmTime;
int pendingPulses;
int minute;
int hour;
int randomNumber;
uint8_t batteryLevel;

// Sampling period in minutes, keep these two definitions in sync. The value
// must be a factor of 60 to get equally spaced samples.
// Possible values: 1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30
// Different values for different battery levels.
const uint8_t samplings[3] = {12, 4, 2};


struct Action {
  unsigned long ms; // ms after the loop start when to run the action
  uint8_t (*action)(); // function (action) to call
  bool (*filter)(); // filter function, see documentation below
  const char* name; // For debugging purpuses only
};


// Filter functions, to be used in the actions table below.
// These functions return true if the action is to be done, false if not.
// The decission is based in battery level and/or time.
bool sendFramesFilter()
{
  return (
    (batteryLevel > 75 && minute == 0) ||                // Once an hour
    (batteryLevel > 65 && minute == 0 && hour % 3 == 0)  // Once every 3 hours
  );
}

bool sendUnsentFilter()
{
  return (batteryLevel > 75 && hour == 13 && minute == 0);
}

bool syncTimeFilter()
{
  return (batteryLevel > 75 && hour == 13 && minute == 0);
}

// Array of actions, must be ordered by ms, will be executed in order.
//
// The length of the loop is defined by the long warm up time of the sensirion
// (10s). So the first action is to warm up the sensirion, and the last ones
// are to read the sensirion and then make its frame.
//
const uint8_t nActions = 18;
Action actions[nActions] = {
  {    0, &WaspUIO::warmSensirion,        NULL,              "Warm Sensirion"},
  {  100, &WaspUIO::createArchiveFile,    NULL,              "Create archive file"},
  {  300, &WaspUIO::warmPressure,         NULL,              "Warm Pressure"},
  {  350, &WaspUIO::warmPressure,         NULL,              "Read Pressure"},
  {  400, &WaspUIO::warmLeafWetness,      NULL,              "Warm LeafWetness"},
  {  450, &WaspUIO::warmLeafWetness,      NULL,              "Read LeafWetness"},
//{  500, &WaspUIO::warmAnemometer,       NULL,              "Warm Anemometer"},
//{  550, &WaspUIO::warmAnemometer,       NULL,              "Read Anemometer"},
//{  600, &WaspUIO::warmVane,             NULL,              "Warm Vane"},
//{  650, &WaspUIO::warmVane,             NULL,              "Read Vane"},
//{  700, &WaspUIO::warmTempDS18B20,      NULL,              "Warm TempDS18B20"},
//{  750, &WaspUIO::warmTempDS18B20,      NULL,              "Read TempDS18B20"},
  { 1000, &WaspUIO::warmRTC,              NULL,              "Warm RTC"},
  { 1050, &WaspUIO::warmRTC,              NULL,              "Read RTC"},
  { 1100, &WaspUIO::warmACC,              NULL,              "Warm ACC"},
  { 1150, &WaspUIO::warmACC,              NULL,              "Read ACC"},
  { 2000, &WaspUIO::frameHealth,          NULL,              "Create Health frame"},
  { 2100, &WaspUIO::framePressureWetness, NULL,              "Create Pressure/Wetness frame"},
  { 2000, &WaspUIO::frameWind,            NULL,              "Create Wind frame"},
  { 3000, &WaspUIO::sendFrames,           &sendFramesFilter, "Send frames"},
  { 4000, &WaspUIO::sendFramesUnsent,     &sendUnsentFilter, "Send frames (unsent)"},
  { 5000, &WaspUIO::receiveGPSsyncTime,   &syncTimeFilter,   "Sync time from GPS"},
  {10000, &WaspUIO::readSensirion,        NULL,              "Read Sensirion"},
  {10100, &WaspUIO::frameSensirion,       NULL,              "Create Sensirion frame"}
};


const uint8_t getSampling() {
  if (batteryLevel <= 30)
  {
    return samplings[0];
  }

  if (batteryLevel <= 55)
  {
    return samplings[1];
  }

  return samplings[2];
}


void setup()
{
  UIO.initTime();

  // Initialize variables, from EEPROM (USB print, OTA programming, ..)
  UIO.initVars();

  // Log
  UIO.start_RTC_SD_USB(false);
  batteryLevel = PWR.getBatteryLevel();
  UIO.logActivity(F("INFO <<< Booting. Agr board ON. Battery level is %d"), batteryLevel);

  // Interactive mode
  UIO.interactive();

  // Create files in SD
  error = UIO.initSD();
  UIO.targetUnsentFile = UIO.unsent_fileA;

  // set random seed
  //srandom(42);

  //UIO.readOwnMAC();

  // Calculate first alarm (requires batteryLevel)
  alarmTime = UIO.getNextAlarm(getSampling());

  // Go to sleep
  UIO.logActivity(F("INFO Boot done in %d ms"), UIO.millisDiff(UIO.start, millis()));
  UIO.stop_RTC_SD_USB();
  PWR.deepSleep(alarmTime, RTC_ABSOLUTE, RTC_ALM1_MODE4, ALL_OFF);
}


void loop()
{
  uint8_t i;
  Action* action;

  UIO.initTime();
  UIO.start_RTC_SD_USB(false);

  // Update RTC time at least once. Kepp minute and hour for later.
  RTC.breakTimeAbsolute(UIO.getTime(), &UIO.time);
  minute = UIO.time.minute;
  hour = UIO.time.hour;

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

    unsigned long start = millis();
    unsigned long diff;
    i = 0;
    while (i < nActions)
    {
      diff = UIO.millisDiff(start, millis());
      action = &actions[i];

      // Filter
      if (action->filter != NULL && action->filter() == false)
      {
        i++;
        continue;
      }

      // Action
      if (action->ms < diff)
      {
        UIO.logActivity("DEBUG Action %s", action->name);
        error = action->action();
        i++;
      }
    }
  }
  else
  {
    UIO.logActivity(F("WARN Unexpected interruption %d"), intFlag);
  }

sleep:
  // Calculate first alarm (requires batteryLevel)
  alarmTime = UIO.getNextAlarm(getSampling());

  UIO.logActivity("INFO Loop done in %lu ms.", UIO.millisDiff(UIO.start, millis()));
  UIO.stop_RTC_SD_USB();

  // Clear interruption flag & pin
  clearIntFlag();
  PWR.clearInterruptionPin();

  // Set whole agri board and waspmote to sleep, until next alarm.
  SensorAgrv20.sleepAgr(alarmTime, RTC_ABSOLUTE, RTC_ALM1_MODE4, ALL_OFF);
}
