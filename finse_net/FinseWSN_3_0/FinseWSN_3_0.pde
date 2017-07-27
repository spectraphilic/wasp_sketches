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

// Sampling period in minutes, keep these two definitions in sync. The value
// must be a factor of 60 to get equally spaced samples.
// Possible values: 1, 2, 3, 4, 5, 6, 10, 12, 15, 20, 30
// Different values for different battery levels.
const uint8_t samplings[] PROGMEM = {15, 10, 5};


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
    UIO.featureNetwork &&
    UIO.time.hour % 2 == 0 // XXX Every 2h
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

const uint8_t getSampling() {
  uint8_t i = 2;

  if (UIO.batteryLevel <= 30)
  {
    i = 0;
  }
  else if (UIO.batteryLevel <= 40)
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
    actionGps();
  }

  // Boot
  UIO.logActivity(F("INFO *** Booting (setup). Battery level is %d"), UIO.batteryLevel);

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
  bool run;
  Action action;

  // Time
  UIO.initTime();
  UIO.start_RTC_SD_USB(false);
  UIO.openFiles();

  // Initialize the action table
  UIO.reset();
  UIO.schedule(ACTION_SENSORS, 1);
  UIO.schedule(ACTION_FRAME_HEALTH, 100);
  // Network
  if (filter_network())
  {
    UIO.schedule(ACTION_NETWORK, 50);
  }
  // GPS
  if (filter_gps())
  {
    UIO.schedule(ACTION_GPS, 9000);
  }

  // Update RTC time at least once. Keep minute and hour for later.
  RTC.breakTimeAbsolute(UIO.getEpochTime(), &UIO.time);

  // Check RTC interruption
  if (intFlag & RTC_INT)
  {
    // Battery level, do nothing if too low
    if (UIO.batteryLevel <= 30) {
      UIO.logActivity(F("DEBUG RTC interruption, low battery = %d"), UIO.batteryLevel);
      goto sleep;
    }

    UIO.logActivity(F("INFO *** RTC interruption, battery level = %d"), UIO.batteryLevel);
    //Utils.blinkGreenLED(); // blink green once every minute to show it is alive

    unsigned long start = millis();
    unsigned long diff;
    unsigned long t0;
    do
    {
      run = false;
      diff = UIO.millisDiff(start);

      // Iter through the actions table
      for (uint8_t i=0; i < nActions; i++)
      {
        uint16_t ms = UIO.actionsQ[i];
	if (ms > 0)
	{
	  run = true;
	  if (ms <= diff)
	  {
	    UIO.actionsQ[i] = 0; // Un-schedule
            memcpy_P(&action, &actions[i], sizeof action);
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
	}
      }

      // Network (receive)
      if (xbeeDM.XBee_ON && xbeeDM.available())
      {
        UIO.logActivity(F("DEBUG New packet available"));
        UIO.receivePacket();
      }

    }
    while (run);
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
