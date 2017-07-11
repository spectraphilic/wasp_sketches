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

bool filter_20min()
{
  return ((minute == 0) || (minute == 20) || (minute == 40));
}


bool sendFramesFilter()
{
  if (! filter_1h()) { return false; } // Net ops happen once/hour at most
  if (! filter_20min()) { return false; } // Net ops happen once/hour at most
  return (
    (batteryLevel > 75) ||                // Once an hour
    (batteryLevel > 65 && hour % 3 == 0)  // Once every 3 hours
  );
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
const char action_01[] PROGMEM = "Warm RTC";
const char action_02[] PROGMEM = "Read RTC";
const char action_03[] PROGMEM = "Warm ACC";
const char action_04[] PROGMEM = "Read ACC";
const char action_05[] PROGMEM = "Create Health frame";
const char action_06[] PROGMEM = "Read LeafWetness";
const char action_07[] PROGMEM = "Read TempDS18B20";
const char action_08[] PROGMEM = "Turn on the Atmospheric Pressure Sensor";
const char action_09[] PROGMEM = "Read Pressure";
const char action_10[] PROGMEM = "Turn off the Atmospheric Pressure Sensor";
const char action_11[] PROGMEM = "Create Pressure/Wetness frame";
const char action_12[] PROGMEM = "Turn on the Meteorology Group";
const char action_13[] PROGMEM = "Read Anemometer";
const char action_14[] PROGMEM = "Read Vane";
const char action_15[] PROGMEM = "Turn off the Meteorology Group";
const char action_16[] PROGMEM = "Create Wind frame";
const char action_17[] PROGMEM = "Start network";
const char action_18[] PROGMEM = "Send frames";
const char action_19[] PROGMEM = "Stop network";
const char action_20[] PROGMEM = "Read Sensirion";
const char action_21[] PROGMEM = "Create Sensirion frame";
const char action_22[] PROGMEM = "Turn off the Low Consumption Group";
const char action_23[] PROGMEM = "Turn off the Agri board";

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
};

const uint8_t nActions = 24;
Action actions[nActions] = {
  {    0, &WaspUIO::onLowConsumptionGroup,  NULL},
  // Frame: Health
  {  100, &WaspUIO::onRTC,                  NULL},
  {  150, &WaspUIO::readRTC,                NULL},
  {  200, &WaspUIO::onACC,                  NULL},
  {  250, &WaspUIO::readACC,                NULL},
  {  300, &WaspUIO::frameHealth,            NULL},
  // Frame: Pressure & Wetness
  {  400, &WaspUIO::readLeafWetness,        NULL},
  {  450, &WaspUIO::readTempDS18B20,        &filter_never},
  {  500, &WaspUIO::onPressureSensor,       NULL},
  {  600, &WaspUIO::readPressure,           NULL},
  {  650, &WaspUIO::offPressureSensor,      NULL},
  {  700, &WaspUIO::framePressureWetness,   NULL},
  // Frame: Wind
  {  800, &WaspUIO::onMeteorologyGroup,     &filter_never},
  {  900, &WaspUIO::readAnemometer,         &filter_never},
  { 1000, &WaspUIO::readVane,               &filter_never},
  { 1050, &WaspUIO::offMeteorologyGroup,    &filter_never},
  { 1100, &WaspUIO::frameWind,              &filter_never},
  // The network window (6s)
  { 2000, &WaspUIO::startNetwork,           &filter_20min},
  { 3000, &WaspUIO::sendFrames,             &sendFramesFilter},
  { 8000, &WaspUIO::stopNetwork,            &filter_20min},
  // Frame: Sensirion
  {10000, &WaspUIO::readSensirion,          NULL},
  {10100, &WaspUIO::frameSensirion,         NULL},
  {10150, &WaspUIO::offLowConsumptionGroup, NULL},
  {10200, &WaspUIO::offAgrBoard,            NULL},
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

  // Hard-code behaviour. Uncomment this if you do not wish to initialize
  // interactively.
  //UIO.updateEEPROM(EEPROM_UIO_FLAGS, FLAG_USB_OUTPUT);
  //UIO.initNet(NETWORK_BROADCAST);

  // Initialize variables, from EEPROM (USB print, OTA programming, ..)
  UIO.initVars();

  // Log
  UIO.start_RTC_SD_USB(false);
  batteryLevel = PWR.getBatteryLevel();
  UIO.logActivity(F("INFO *** Booting (setup). Battery level is %d"), batteryLevel);

  // Interactive mode
  UIO.interactive();

  // Create files in SD
  error = UIO.initSD();

  // Set random seed, different for every device
  srandom(Utils.readSerialID());

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
  RTC.breakTimeAbsolute(UIO.getEpochTime(), &UIO.time);
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
    USB.println("before ON");
    SensorAgrv20.ON();
    USB.println("after ON");

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
        UIO.logActivity(F("DEBUG New packet available"));
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
  PWR.deepSleep(alarmTime, RTC_ABSOLUTE, RTC_ALM1_MODE4, ALL_OFF);
}
