/******************************************************************************
 * Includes
 ******************************************************************************/

#include "WaspUIO.h"
#include <assert.h>


/******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * Function to be called first in setup()
 */
void WaspUIO::boot()
{
  nloops = 0;
  SdFile::dateTimeCallback(WaspUIO::dateTime);

  RTC.ON();
  cr.print(F("."));
  UIO.onSetup();
  cr.print(F("."));
  UIO.onLoop();
  cr.print(F("."));
  UIO.networkInit(); // Network
}


void WaspUIO::onSetup()
{
  /*** 1. Read configuration from EEPROM ***/
  // Flags
  flags = Utils.readEEPROM(EEPROM_UIO_FLAGS);

  networkType = (network_type_t) Utils.readEEPROM(EEPROM_UIO_NETWORK_TYPE);
  if (networkType >= NETWORK_LEN) { networkType = NETWORK_XBEE; }

#if WITH_CRYPTO
  // Frame encryption
  UIO.readEEPROM(EEPROM_UIO_PWD, password, sizeof password);
  size_t len = strlen(password);
  if (len != 0 && len != 16 && len != 24 && len != 32)
  {
    password[0] = '\0';
  }
#endif

#if WITH_4G
  // 4G network
  char apn[30];
  pin = eeprom_read_word((uint16_t*)EEPROM_UIO_PIN);
  UIO.readEEPROM(EEPROM_UIO_APN, apn, sizeof apn);
  _4G.set_APN(apn);
#endif

#if WITH_XBEE
  // XBee network
  uint8_t panid_low = Utils.readEEPROM(EEPROM_UIO_XBEE+1);
  if (panid_low >= xbee_len) { panid_low = 2; } // Default
  memcpy_P(&xbee, &xbees[panid_low], sizeof xbee);
#endif

  // Read run table
  for (uint8_t i=0; i < RUN_LEN; i++)
  {
    const char* name = (const char*)pgm_read_word(&(run_names[i]));
    if (strcmp_P("", name) == 0)
    {
      actions[i] = 0;
    }
    else
    {
      uint16_t base = EEPROM_UIO_RUN + (i * 2);
      uint8_t hours = Utils.readEEPROM(base);
      uint8_t minutes = Utils.readEEPROM(base + 1);
      actions[i] = (hours * 60) + minutes;
    }
  }

  // Frames
  setFrameSize();

  // Log level
  cr.loglevel = (loglevel_t) Utils.readEEPROM(EEPROM_UIO_LOG_LEVEL);

  /*** 2. Autodetect hardware ***/
  hasGPS = GPS_NO;
#if WITH_4G
  if (_4G.ON() == 0)
  {
    hasGPS |= GPS_4G;
    _4G.OFF();
  }
#endif
#if WITH_GPS
  if (GPS.ON() == 1)
  {
    hasGPS |= GPS_YES;
    GPS.OFF();
  }
#endif

  // USB autodetect
  // Cannot be done with ATMega1281 but it can be with ATmega328P
}

void WaspUIO::onLoop()
{
  readBattery();

  pinMode(PIN_1WIRE, INPUT);
  pinMode(PIN_SDI12, INPUT);

  startSD();
  loadTime();
}


/**
 * Retturn true if the given sensor is to be read now.
 */
bool WaspUIO::action(uint8_t n, ...)
{
  va_list args;
  bool yes = false;

  uint32_t minutes = epochTime / 60; // minutes since the epoch

  va_start(args, n);
  for (; n; n--)
  {
    int idx = va_arg(args, int);
    assert(idx < RUN_LEN); // TODO Define __assert

    uint16_t value = actions[idx] * cooldown;
    if (value > 0)
    {
      if (minutes % value == 0)
      {
        yes = true;
        break;
      }
    }
  }
  va_end(args);

  return yes;
}


/******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/

/**
 * Read a line from the given open file, not including the end-of-line
 * character. Store the read line in SD.buffer.
 *
 * Return the length of the line. Or -1 for EOF. Or -2 if error.
 */
void WaspUIO::getDataFilename(char* filename, uint8_t year, uint8_t month, uint8_t date)
{
  sprintf(filename, "%s/%02u%02u%02u.TXT", archive_dir, year, month, date);
}

/// Preinstantiate Objects /////////////////////////////////////////////////////
WaspUIO UIO = WaspUIO();
FIFO fifo = FIFO("TMP.TXT", "QSTART.BIN", 8); // TODO Rename to FIFO.BIN / FIDX.BIN
#if WITH_IRIDIUM
IridiumSBD iridium(Serial1, DIGITAL4); // RING pins not connected
LIFO lifo = LIFO("LIFO.BIN", 8);
#endif


/**
 * getNextAlarm
 *
 * Return the string of the next alarm.
 *
 */
uint32_t WaspUIO::nextAlarm(char* alarmTime)
{
  uint32_t next = ULONG_MAX / 60; // max posible value of minutes since epoch

  // Minutes since epoch
  // With +2s safeguard, this means we've 2s before going to sleep
  uint32_t epoch = getEpochTime();
  uint32_t minutes = (epoch + 2) / 60;

  for (uint8_t i=0; i < RUN_LEN; i++)
  {
    uint32_t value = actions[i] * cooldown;
    if (value > 0)
    {
      value = (minutes / value + 1) * value;
      if (value < next)
      {
        next = value;
      }
    }
  }

  // Format time to string, to be passed to deepSleep
  timestamp_t ts;
  RTC.breakTimeAbsolute(next * 60, &ts);
  sprintf(alarmTime, "00:%02d:%02d:00", ts.hour, ts.minute);
  //sprintf(alarmTime, "%02d:%02d:%02d:00", ts.date, ts.hour, ts.minute);
  return next - minutes;
}

void WaspUIO::deepSleep()
{
  saveTimeToSD();

  // For robustness sake, reboot once in a while
  bool reboot = (nloops >= MAX_LOOPS);
  if (reboot)
  {
    info(F("Rebooting after %u loops"), nloops);
  }

  // Get next alarm time
  char alarmTime[12]; // "00:00:00:00"
  uint32_t next = nextAlarm(alarmTime);
  debug(F("Next alarm at %s in %lu minutes"), alarmTime, next);

  // Stop SD, logging ends here
  UIO.stopSD();

  // Clear interruption flag & pin
  intFlag = 0;
  PWR.clearInterruptionPin();

  // Reboot
  if (reboot)
  {
    PWR.switchesOFF(ALL_OFF);
    PWR.reboot();
  }

  // Reset watchdog
  if (_boot_version >= 'H')
  {
    // Max is 43200 (30 days), for alarm 2.
    // Though in this example it says max is 1000
    // http://www.libelium.com/development/waspmote/examples/rtc-10-set-watchdog/
    // Anyway, better not to sleep for tooo long. We cap it to 960 (16h)
    if (next > 960)
    {
      next = 960;
    }

    if (RTC.setWatchdog(next + 2))
    {
      PWR.switchesOFF(ALL_OFF);
      PWR.reboot();
    }
  }

  // Power off and Sleep
  // XXX Using MODE3 instead of MODE2 to not sleep for more than 1 day
  PWR.deepSleep(alarmTime, RTC_ABSOLUTE, RTC_ALM1_MODE3, ALL_OFF);
  nloops++;

  // Awake: Reset if stuck for 4 minutes
  if (_boot_version >= 'H')
  {
    if (RTC.setWatchdog(LOOP_TIMEOUT))
    {
      PWR.switchesOFF(ALL_OFF);
      PWR.reboot();
    }
  }
}
