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

  boardType = (board_type_t) Utils.readEEPROM(EEPROM_UIO_BOARD_TYPE);
  if (boardType >= BOARD_LEN) { boardType = BOARD_NONE; }

  batteryType = (battery_type_t) Utils.readEEPROM(EEPROM_UIO_BATTERY_TYPE);
  if (batteryType >= BATTERY_LEN) { batteryType = BATTERY_LITHIUM; }

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
#if WITH_GPS
  hasGPS = GPS.ON();
  GPS.OFF();
#endif

  // USB autodetect
  // Cannot be done with ATMega1281 but it can be with ATmega328P
}

void WaspUIO::onLoop()
{
  cr.sleep_time = 0;
  state = 0;

  RTC.ON(); // SD calls the RTC to set file/dir time
  loadTime();
  UIO.startSD();

  if (batteryType == BATTERY_LEAD) { startPowerBoard(); }
  if (boardType == BOARD_LEMMING) { startSensorBoard(); }

  readBattery();
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


/**
 * getNextAlarm
 *
 * Return the string of the next alarm.
 *
 */
int WaspUIO::nextAlarm(char* alarmTime)
{
  int next = INT_MAX;

  uint32_t epoch = getEpochTime();
  uint32_t minutes = (epoch / 60); // minutes since epoch

  for (uint8_t i=0; i < RUN_LEN; i++)
  {
    uint16_t value = actions[i] * cooldown;
    if (value > 0)
    {
      value = (minutes / value + 1) * value - minutes;
      if (value < next)
      {
        next = value;
      }
    }
  }

  // Format relative time to string, to be passed to deepSleep
  int days = next / (24 * 60);
  int left = next - (days * 24 * 60);
  int hours = left / 60;
  left = left - (hours * 60);

  sprintf(alarmTime, "%02d:%02d:%02d:00", days, hours, left);
  return next;
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
  UIO.stopSD();

  // Clear interruption flag & pin
  intFlag = 0;
  PWR.clearInterruptionPin();

  // Reboot
  if (reboot) { PWR.reboot(); }

  // Sleep
  // Get next alarm time
  char alarmTime[12]; // "00:00:00:00"
  int next = nextAlarm(alarmTime);

  // Reset watchdog
  if (_boot_version >= 'H')
  {
    if (next < 59) // XXX Maximum is 59
    {
      RTC.setWatchdog(next + 1);
    }
  }

  // Power off and Sleep
  //i2c(0); maxbotix(0); onewire(0); sdi12(0); v33(0); v5(0);
  if (batteryType == BATTERY_LEAD)
  {
    leadVoltage(0); v12(0);
  }
  PWR.deepSleep(alarmTime, RTC_OFFSET, RTC_ALM1_MODE2, ALL_OFF);
  nloops++;

  // Awake: Reset if stuck for 4 minutes
  if (_boot_version >= 'H')
  {
    RTC.setWatchdog(UIO.loop_timeout);
  }
}
