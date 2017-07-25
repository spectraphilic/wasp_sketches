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

bool filter_network()
{
  return (
    UIO.featureNetwork &
    UIO.time.minute % 20 == 0 // XXX Every 20 minutes, should be 1h in deployment
  );
}

bool filter_sdi12()
{
  return (UIO.sensors & SENSOR_SDI12_CTD10);
}

bool filter_pressure()
{
  return (UIO.sensors & SENSOR_AGR_PRESSURE);
}

bool filter_lw()
{
  return (UIO.sensors & SENSOR_AGR_LEAFWETNESS);
}

bool filter_sensirion()
{
  return (UIO.sensors & SENSOR_AGR_SENSIRION);
}

bool sendFramesFilter()
{
  if (! UIO.hasSD)
  {
    return false;
  }

  if (! filter_network()) { return false; } // Net ops happen once/hour at most
  return (
    (batteryLevel > 75) ||                // Once an hour
    (batteryLevel > 65 && UIO.time.hour % 3 == 0)  // Once every 3 hours
  );
}

// Array of actions, must be ordered by ms, will be executed in order.
//

const Action actions[] PROGMEM = {
  {    0, &WaspUIO::sensorsPowerOn,         NULL,              "Sensors power On"},          // ~11ms
  {    0, &WaspUIO::startNetwork,           &filter_network,   "Start network"},             // ~557ms ?
  // Internal sensors (health)
  //{  100, &WaspUIO::readACC,                NULL,              "Read ACC"},                // ~37ms
  {  200, &WaspUIO::frameHealth,            NULL,              "Create Health frame"},       // ~133ms
  // SDI-12
  {  500, &WaspUIO::SDI12_on,               &filter_sdi12,     "SDI-12 turn ON"}, // ~500ms after 5V on
  {  600, &WaspUIO::SDI12_CTD10_measure,    &filter_sdi12,     "SDI-12 CTD10, send Measure command"},
  { 1200, &WaspUIO::SDI12_CTD10_data,       &filter_sdi12,     "SDI-12 CTD10, read data"}, // ~800ms after measure
  //{  600, &WaspUIO::SDI12_DS2_measure,    &filter_ds2,         "SDI-12 DS2, send Measure command"},
  //{ 1200, &WaspUIO::SDI12_DS2_data,       &filter_ds2,         "SDI-12 DS2, read data"},
  { 1300, &WaspUIO::SDI12_off,              &filter_sdi12,     "SDI-12 turn OFF"},
  { 1400, &WaspUIO::SDI12_CTD10_frame,      &filter_sdi12,     "SDI-12 CTD10 Create frame"},
  // Agr: Pressure
  {  500, &WaspUIO::Agr_Pressure,           &filter_pressure,  "Read Pressure"},             // ~51ms
  {  700, &WaspUIO::framePressure,          &filter_pressure,  "Create Pressure frame"},     // ~114ms
  // Agr: Wind
  //{  500, &WaspUIO::Agr_Meteo_Anemometer,   NULL,     "Read Anemometer"},
  //{  700, &WaspUIO::Agr_Meteo_Vane,         NULL,     "Read Vane"},
  //{  900, &WaspUIO::frameWind,              NULL,     "Create Wind frame"},
  // Agr: Low consumption group
  {  500, &WaspUIO::Agr_LCGroup_LeafWetness, &filter_lw,       "Read LeafWetness"},          // ~51ms
  //{  600, &WaspUIO::Agr_LCGroup_SoilTemp,   NULL,     "Read TempDS18B20"},
  {  900, &WaspUIO::Agr_LCGroup_Sensirion,  &filter_sensirion, "Read Sensirion"},            // ~356ms
  { 1100, &WaspUIO::frameLeafWetness,       &filter_lw,        "Create Leaf Wetness frame"}, // ~152ms
  { 1300, &WaspUIO::frameSensirion,         &filter_sensirion, "Create Sensirion frame"},    // ~119ms
  // Turn Off sensors, as soon as possible
  {    0, &WaspUIO::sensorsPowerOff,        NULL,              "Sensors power Off"},         // ~43ms ?
  // The network window (8s minimum)
  {    0, &WaspUIO::sendFrames,             &sendFramesFilter, "Send frames"},               // ~59ms / frame
  { 8000, &WaspUIO::stopNetwork,            &filter_network,   "Stop network"},              // ~43ms ?
  // GPS (once a day)
  { 9000, &WaspUIO::setTimeFromGPS,         &filter_gps,       "Set RTC time from GPS"},
};
const uint8_t nActions = sizeof actions / sizeof actions[0];


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

  // Interactive mode
  UIO.start_RTC_SD_USB(false);
  UIO.interactive();

  // Create/Open files
  error = UIO.initSD();
  UIO.openFiles();

  // Set time from GPS if wrong time is detected
  // XXX Do this unconditionally to update location?
  if (UIO.epochTime < 1483225200) // 2017-01-01 arbitrary date in the past
  {
    UIO.logActivity(F("WARN Wrong time detected, updating from GPS"));
    UIO.setTimeFromGPS();
  }

  // Boot
  batteryLevel = PWR.getBatteryLevel();
  UIO.logActivity(F("INFO *** Booting (setup). Battery level is %d"), batteryLevel);

  // Set random seed, different for every device
  srandom(Utils.readSerialID());

  //UIO.readOwnMAC();

  // Calculate first alarm (requires batteryLevel)
  alarmTime = UIO.getNextAlarm(getSampling());

  // Go to sleep
  UIO.logActivity(F("INFO Boot done, go to sleep"));
  UIO.closeFiles();
  UIO.stop_RTC_SD_USB();
  PWR.deepSleep(alarmTime, RTC_ABSOLUTE, RTC_ALM1_MODE4, ALL_OFF);
}


void loop()
{
  uint8_t i;
  Action action;

  UIO.initTime();
  UIO.start_RTC_SD_USB(false);
  UIO.openFiles();

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

    UIO.logActivity(F("INFO *** RTC interruption, battery level = %d"), batteryLevel);
    //Utils.blinkGreenLED(); // blink green once every minute to show it is alive

    unsigned long start = millis();
    unsigned long diff;
    unsigned long t0;
    i = 0;
    while (i < nActions)
    {
      diff = UIO.millisDiff(start);
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
        t0 = millis();
        //UIO.logActivity(F("DEBUG Action %s: start"), action.name);
        if (action.action())
        {
           UIO.logActivity(F("ERROR Action %s: error"), action.name);
        }
        else
        {
           UIO.logActivity(F("DEBUG Action %s: done in %lu ms"), action.name, UIO.millisDiff(t0));
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

  UIO.logActivity(F("INFO Loop done in %lu ms."), UIO.millisDiff(UIO.start));
  //UIO.print(F("LOOP %lu"), UIO.millisDiff(UIO.start));
  UIO.closeFiles();
  UIO.stop_RTC_SD_USB();

  // Clear interruption flag & pin
  clearIntFlag();
  PWR.clearInterruptionPin();

  // Set whole agri board and waspmote to sleep, until next alarm.
  PWR.deepSleep(alarmTime, RTC_ABSOLUTE, RTC_ALM1_MODE4, ALL_OFF);
}
