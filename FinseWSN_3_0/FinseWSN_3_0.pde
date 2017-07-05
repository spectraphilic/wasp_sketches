/*
 SCRIPT for Finse network, to synchronize DM network, and read basic set of sensors
 April 2017, Simon Filhol

 Script description:

 */

// 1. Include Libraries
#include <WaspUIO.h>
#include <WaspSensorAgr_v20.h>
#include <WaspFrame.h>
#include <WaspXBeeDM.h>

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
const uint8_t samplings[] PROGMEM = {12, 4, 2};


struct Action {
  unsigned long ms; // ms after the loop start when to run the action
  uint8_t (*action)(); // function (action) to call
  bool (*filter)(); // filter function, see documentation below
};


// Filter functions, to be used in the actions table below.
// These functions return true if the action is to be done, false if not.
// The decission is based in battery level and/or time.
bool filter_never()
{
  return false;
}


bool filter_1h()
{
  return (minute == 0);
}

bool sendFramesFilter()
{
  if (! filter_1h()) { return false; } // Net ops happen once/hour at most
  return (
    (batteryLevel > 75) ||                // Once an hour
    (batteryLevel > 65 && hour % 3 == 0)  // Once every 3 hours
  );
}

bool sendUnsentFilter()
{
  if (! filter_1h()) { return false; } // Net ops happen once/hour at most
  return (batteryLevel > 75 && hour == 13);
}

// Array of actions, must be ordered by ms, will be executed in order.
//
// The length of the loop is defined by the long warm up time of the sensirion
// (10s). So the first action is to warm up the sensirion, and the last ones
// are to read the sensirion and then make its frame. This means that the
// sensirion frame will be sent in the next loop, always one loop behind the
// others.
//

char buffer[60];
const char action_00[] PROGMEM = "Turn on the Low Consumption Group";
const char action_01[] PROGMEM = "Create archive file";
const char action_02[] PROGMEM = "Warm RTC";
const char action_03[] PROGMEM = "Read RTC";
const char action_04[] PROGMEM = "Warm ACC";
const char action_05[] PROGMEM = "Read ACC";
const char action_06[] PROGMEM = "Create Health frame";
const char action_07[] PROGMEM = "Read LeafWetness";
const char action_08[] PROGMEM = "Read TempDS18B20";
const char action_09[] PROGMEM = "Turn on the Atmospheric Pressure Sensor";
const char action_10[] PROGMEM = "Read Pressure";
const char action_11[] PROGMEM = "Turn off the Atmospheric Pressure Sensor";
const char action_12[] PROGMEM = "Create Pressure/Wetness frame";
const char action_13[] PROGMEM = "Turn on the Meteorology Group";
const char action_14[] PROGMEM = "Read Anemometer";
const char action_15[] PROGMEM = "Read Vane";
const char action_16[] PROGMEM = "Turn off the Meteorology Group";
const char action_17[] PROGMEM = "Create Wind frame";
const char action_18[] PROGMEM = "Start network";
const char action_19[] PROGMEM = "Send frames";
const char action_20[] PROGMEM = "Send frames (unsent)";
const char action_21[] PROGMEM = "Stop network";
const char action_22[] PROGMEM = "Read Sensirion";
const char action_23[] PROGMEM = "Create Sensirion frame";
const char action_24[] PROGMEM = "Turn off the Low Consumption Group";

const char* const action_messages[] PROGMEM = {
  action_00,
  action_01,
  action_02,
  action_03,
  action_04,
  action_05,
  action_06,
  action_07,
  action_08,
  action_09,
  action_10,
  action_11,
  action_12,
  action_13,
  action_14,
  action_15,
  action_16,
  action_17,
  action_18,
  action_19,
  action_20,
  action_21,
  action_22,
  action_23,
  action_24,
};

const uint8_t nActions = 25;
Action actions[nActions] = {
  {    0, &WaspUIO::onLowConsumptionGroup,  NULL},
  {  100, &WaspUIO::createArchiveFile,      NULL},
  // Frame: Health
  {  200, &WaspUIO::onRTC,                  NULL},
  {  250, &WaspUIO::readRTC,                NULL},
  {  300, &WaspUIO::onACC,                  NULL},
  {  350, &WaspUIO::readACC,                NULL},
  {  400, &WaspUIO::frameHealth,            NULL},
  // Frame: Pressure & Wetness
  {  500, &WaspUIO::readLeafWetness,        NULL},
  {  550, &WaspUIO::readTempDS18B20,        &filter_never},
  {  600, &WaspUIO::onPressureSensor,       NULL},
  {  700, &WaspUIO::readPressure,           NULL},
  {  750, &WaspUIO::offPressureSensor,      NULL},
  {  800, &WaspUIO::framePressureWetness,   NULL},
  // Frame: Wind
  {  900, &WaspUIO::onMeteorologyGroup,     &filter_never},
  { 1000, &WaspUIO::readAnemometer,         &filter_never},
  { 1100, &WaspUIO::readVane,               &filter_never},
  { 1150, &WaspUIO::offMeteorologyGroup,    &filter_never},
  { 1200, &WaspUIO::frameWind,              &filter_never},
  // The network window (6s)
  { 2000, &WaspUIO::startNetwork,           &filter_1h},
  { 3000, &WaspUIO::sendFrames,             &sendFramesFilter},
  { 4000, &WaspUIO::sendFramesUnsent,       &sendUnsentFilter},
  { 8000, &WaspUIO::stopNetwork,            &filter_1h},
  // Frame: Sensirion
  {10000, &WaspUIO::readSensirion,          NULL},
  {10100, &WaspUIO::frameSensirion,         NULL},
  {10150, &WaspUIO::offLowConsumptionGroup, NULL},
};


const uint8_t getSampling() {
  uint8_t i = 2;

  if (batteryLevel <= 30)
  {
    i = 0;
  }
  else if (batteryLevel <= 55)
  {
    i = 1;
  }

  return pgm_read_byte_near(samplings + i);
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
  UIO.logActivity(F("INFO Boot done in %lu ms"), UIO.millisDiff(UIO.start, millis()));
  UIO.stop_RTC_SD_USB();
  PWR.deepSleep(alarmTime, RTC_ABSOLUTE, RTC_ALM1_MODE4, ALL_OFF);
}


void loop()
{
  uint8_t i;
  Action* action;

  UIO.initTime();
  UIO.start_RTC_SD_USB(false);

  // Update RTC time at least once. Keep minute and hour for later.
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

    // Sensor board on. Apparently it requires RTC.
    SensorAgrv20.ON();

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
        strcpy_P(buffer, (char*)pgm_read_word(&(action_messages[i])));
        i++;
        UIO.logActivity(F("DEBUG Action %s"), buffer);
        error = action->action();
        if (error)
        {
          UIO.logActivity(F("ERROR Action %s: %d"), buffer, error);
        }
      }

      // Network (receive)
      if (xbeeDM.XBee_ON && xbeeDM.available())
      {
         UIO.logActivity("DEBUG New packet available");
         UIO.receivePacket();
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
