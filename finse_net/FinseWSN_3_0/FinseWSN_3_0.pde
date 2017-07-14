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
int randomNumber;
uint8_t batteryLevel;

// Sampling period in minutes, keep these two definitions in sync. The value
// must be a factor of 60 to get equally spaced samples.
// Possible values: 1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30
// Different values for different battery levels.
const uint8_t samplings[] PROGMEM = {12, 4, 2};


typedef struct {
  unsigned long ms; // ms after the loop start when to run the action
  uint8_t (*action)(); // function (action) to call
  bool (*filter)(); // filter function, see documentation below
  char name[50];
} Action;


// Filter functions, to be used in the actions table below.
// These functions return true if the action is to be done, false if not.
// The decission is based in battery level and/or time.
bool filter_never()
{
  return false;
}


bool filter_gps()
{
  // The RTC is DS3231SN which at -40 C has an accuracy of 3.5ppm, that's
  // about 0.3s per day, with aging it may be worse.
  // See https://www.maximintegrated.com/en/app-notes/index.mvp/id/3566

  // As clock sync between the devices is critical for the network to work
  // properly, we update the RTC time from GPS daily. But the GPS draws power,
  // so this may need to be tuned.
  return (UIO.time.minute == 0 && UIO.time.hour == 0);
}

bool filter_1h()
{
  return (UIO.time.minute % 60 ==  0);
}

bool filter_20min()
{
  return (UIO.time.minute % 20 == 0);
}


bool sendFramesFilter()
{
  //if (! filter_1h()) { return false; } // Net ops happen once/hour at most
  if (! filter_20min()) { return false; } // Net ops happen once/hour at most
  return (
    (batteryLevel > 75) ||                // Once an hour
    (batteryLevel > 65 && UIO.time.hour % 3 == 0)  // Once every 3 hours
  );
}

// Array of actions, must be ordered by ms, will be executed in order.
//

const uint8_t nActions = 30;
const Action actions[nActions] PROGMEM = {
  {    0, &WaspUIO::onLowConsumptionGroup,  NULL,              "Turn on the Low Consumption Group"},
  // Frame: Health
  {  100, &WaspUIO::onRTC,                  NULL,              "Warm RTC"},
  {  150, &WaspUIO::readRTC,                NULL,              "Read RTC"},
  {  200, &WaspUIO::onACC,                  NULL,              "Warm ACC"},
  {  250, &WaspUIO::readACC,                NULL,              "Read ACC"},
  {  300, &WaspUIO::frameHealth,            NULL,              "Create Health frame"}, // ~100ms
  // Frame: Pressure & Wetness
  {  400, &WaspUIO::readLeafWetness,        NULL,              "Read LeafWetness"},
  {  450, &WaspUIO::readTempDS18B20,        &filter_never,     "Read TempDS18B20"},
  {  500, &WaspUIO::onPressureSensor,       NULL,              "Turn on the Atmospheric Pressure Sensor"},
  {  600, &WaspUIO::readPressure,           NULL,              "Read Pressure"},
  {  650, &WaspUIO::offPressureSensor,      NULL,              "Turn off the Atmospheric Pressure Sensor"},
  {  700, &WaspUIO::framePressureWetness,   NULL,              "Create Pressure/Wetness frame"},
  // Frame: Wind
  {  800, &WaspUIO::onMeteorologyGroup,     &filter_never,     "Turn on the Meteorology Group"},
  {  900, &WaspUIO::readAnemometer,         &filter_never,     "Read Anemometer"},
  { 1000, &WaspUIO::readVane,               &filter_never,     "Read Vane"},
  { 1050, &WaspUIO::offMeteorologyGroup,    &filter_never,     "Turn off the Meteorology Group"},
  { 1100, &WaspUIO::frameWind,              &filter_never,     "Create Wind frame"},
  // Frame: Sensirion
  { 1200, &WaspUIO::readSensirion,          NULL,              "Read Sensirion"}, // This is slow, ~312ms
  { 1500, &WaspUIO::frameSensirion,         NULL,              "Create Sensirion frame"},
  { 1600, &WaspUIO::offLowConsumptionGroup, NULL,              "Turn off the Low Consumption Group"},
  { 1700, &WaspUIO::offAgrBoard,            NULL,              "Turn off the Agri board"},
  // Frame: SDI-12 (XXX This requires tunning)
  { 2000, &WaspUIO::SDI12_on,               &filter_never,     "SDI-12 turn ON"},
  { 2100, &WaspUIO::SDI12_CTD10_measure,    &filter_never,     "SDI-12 CTD10, send Measure command"},
  { 2600, &WaspUIO::SDI12_CTD10_data,       &filter_never,     "SDI-12 CTD10, read data"},
  { 2700, &WaspUIO::SDI12_off,              &filter_never,     "SDI-12 turn OFF"},
  { 2900, &WaspUIO::SDI12_CTD10_frame,      &filter_never,     "SDI-12 CTD10 Create frame"},
  // The network window (6s)
  { 3000, &WaspUIO::startNetwork,           &filter_20min,     "Start network"},
  { 4000, &WaspUIO::sendFrames,             &sendFramesFilter, "Send frames"},
  { 9000, &WaspUIO::stopNetwork,            &filter_20min,     "Stop network"},
  // GPS (once a day)
  {10000, &WaspUIO::setTimeFromGPS,         &filter_gps,       "Set RTC time from GPS"},
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

  // Set time from GPS if wrong time is detected
  // XXX Do this unconditionally to update location?
  if (UIO.epochTime < 1483225200) // 2017-01-01 arbitrary date in the past
  {
    UIO.logActivity(F("WARN Wrong time detected, updating from GPS"));
    UIO.setTimeFromGPS();
  }

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

  // SDI-12
  // UIO.SDI12_CTD10_ident();

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
  Action action;

  UIO.initTime();
  UIO.start_RTC_SD_USB(false);

  // Update RTC time at least once. Keep minute and hour for later.
  RTC.breakTimeAbsolute(UIO.getEpochTime(), &UIO.time);

  // Check RTC interruption
  if (intFlag & RTC_INT)
  {
    // Battery level, do nothing if too low
    batteryLevel = PWR.getBatteryLevel();
    if (batteryLevel <= 30) {
      UIO.logActivity(F("DEBUG RTC interruption, low battery = %d"), batteryLevel);
      goto sleep;
    }

    UIO.logActivity(F("INFO RTC interruption, battery level = %d"), batteryLevel);
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
      memcpy_P(&action, &actions[i], sizeof action);

      // Filter
      if (action.filter != NULL && action.filter() == false)
      {
        i++;
        continue;
      }

      // Action
      if (action.ms < diff)
      {
        i++;
        UIO.logActivity(F("DEBUG Action %s"), action.name);
        error = action.action();
        if (error)
        {
          UIO.logActivity(F("ERROR Action %s: %d"), action.name, error);
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

  UIO.logActivity(F("INFO Loop done in %lu ms."), UIO.millisDiff(UIO.start, millis()));
  UIO.stop_RTC_SD_USB();

  // Clear interruption flag & pin
  clearIntFlag();
  PWR.clearInterruptionPin();

  // Set whole agri board and waspmote to sleep, until next alarm.
  PWR.deepSleep(alarmTime, RTC_ABSOLUTE, RTC_ALM1_MODE4, ALL_OFF);
}
