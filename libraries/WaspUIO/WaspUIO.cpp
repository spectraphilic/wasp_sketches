/******************************************************************************
 * Includes
 ******************************************************************************/

#include "WaspUIO.h"
#include <assert.h>


/******************************************************************************
* Constructors
******************************************************************************/

SDI12 mySDI12(PIN_SDI12);


/******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * Function to be called first in setup()
 */
void WaspUIO::onSetup()
{
  /*** 1. Read configuration from EEPROM ***/
  char apn[30];

  // Flags
  flags = Utils.readEEPROM(EEPROM_UIO_FLAGS);

  boardType = (board_type_t) Utils.readEEPROM(EEPROM_UIO_BOARD_TYPE);
  if (boardType >= BOARD_LEN) { boardType = BOARD_NONE; }

  batteryType = (battery_type_t) Utils.readEEPROM(EEPROM_UIO_BATTERY_TYPE);
  if (batteryType >= BATTERY_LEN) { batteryType = BATTERY_LITHIUM; }

  networkType = (network_type_t) Utils.readEEPROM(EEPROM_UIO_NETWORK_TYPE);
  if (networkType >= NETWORK_LEN) { networkType = NETWORK_XBEE; }

  // 4G network
#if WITH_4G
  pin = eeprom_read_word((uint16_t*)EEPROM_UIO_PIN);
  UIO.readEEPROM(EEPROM_UIO_APN, apn, sizeof apn);
  _4G.set_APN(apn);
  UIO.readEEPROM(EEPROM_UIO_PWD, password, sizeof password);
#endif

  // XBee network
  uint8_t panid_low = Utils.readEEPROM(EEPROM_UIO_XBEE+1);
  if (panid_low >= xbee_len) { panid_low = 2; } // Default
  memcpy_P(&xbee, &xbees[panid_low], sizeof xbee);

  // Read run table
  for (uint8_t i=0; i < RUN_LEN; i++)
  {
    actions[i] = Utils.readEEPROM(EEPROM_UIO_RUN + i);
  }

  // Log level
  cr.loglevel = (loglevel_t) Utils.readEEPROM(EEPROM_UIO_LOG_LEVEL);

  /*** 2. Autodetect hardware ***/
  hasSD = SD.ON();
  SD.OFF();
  hasGPS = GPS.ON();
  GPS.OFF();

  // USB autodetect
  // Cannot be done with ATMega1281 but it can be with ATmega328P
}

void WaspUIO::onLoop()
{
  cr.sleep_time = 0;
  state = 0;

  loadTime(true); // Read temperature as well
  uint32_t epoch = getEpochTime();
  // Split the epoch time in 2: days since the epoch, and minute of the day
  day = epoch / (24L * 60L * 60L);
  minute = (epoch / 60) % (60 * 24); // mins since the epoch modulus mins in a day

  if (batteryType == BATTERY_LEAD) { startPowerBoard(); }
  if (boardType == BOARD_LEMMING) { startSensorBoard(); }

  readBattery();
}


/**
 * Functions to start and stop SD. Open and closes required files.
 */

void WaspUIO::startSD()
{
  if (hasSD)
  {
    if (! (WaspRegister & REG_SD))
    {
      SD.ON();
      baselayout();
    }
  }
}

void WaspUIO::stopSD()
{
  if (SPI.isSD)
  {
    // Close files
    if (logFile.isOpen()) { logFile.close(); }
    if (queueFile.isOpen()) { queueFile.close(); }
    if (qstartFile.isOpen()) { qstartFile.close(); }
    // Off
    SD.OFF();
  }
}


/**
 * Retturn true if the given sensor is to be read now.
 */
bool WaspUIO::action(uint8_t n, ...)
{
  va_list args;
  int idx;
  int value;
  bool yes = false;

  va_start(args, n);
  for (; n; n--)
  {
    idx = va_arg(args, int);
    assert(idx < RUN_LEN); // TODO Define __assert
    value = (int) actions[idx];
    if (value > 0)
    {
      if (minute % (cooldown * value) == 0)
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
void WaspUIO::nextAlarm()
{
  int value;
  int next = INT_MAX;

  for (uint8_t i=0; i < RUN_LEN; i++)
  {
    value = (int) actions[i];
    if (value > 0)
    {
      value *= cooldown;
      value = (minute / value) * value + value;
      if (value < next)
      {
        next = value;
      }
    }
  }

  if (next >= (24 * 60))
    next = 0;

  next_minute = next;
}

const char* WaspUIO::nextAlarm(char* alarmTime)
{
  uint32_t epoch = getEpochTime();
  minute = (epoch / 60) % (60 * 24); // minute of the day
  nextAlarm();

  // Format relative time to string, to be passed to deepSleep
  sprintf(alarmTime, "00:%02d:%02d:00", next_minute / 60, next_minute % 60);
  return alarmTime;
}

void WaspUIO::deepSleep()
{
  // Clear interruption flag & pin
  intFlag = 0;
  PWR.clearInterruptionPin();

  // Get next alarm time
  char alarmTime[12]; // "00:00:00:00"
  nextAlarm(alarmTime);

  // Reset watchdog
  int left = (next_minute == 0)? 1440 - minute: next_minute - minute;
  if (left < 59) // XXX Maximum is 59
  {
    RTC.setWatchdog(left + 1);
  }

  // Turn off sensor & power boards
  if (boardType == BOARD_LEMMING)
  {
    i2c(0); maxbotix(0); onewire(0); sdi12(0);
  }
  if (batteryType == BATTERY_LEAD)
  {
    leadVoltage(0); v33(0); v5(0); v12(0);
  }

  // Enable RTC interruption and sleep
  PWR.deepSleep(alarmTime, RTC_ABSOLUTE, RTC_ALM1_MODE3, ALL_OFF);

  // Awake: Reset if stuck for 4 minutes
  RTC.setWatchdog(UIO.loop_timeout);
}


/**
 * To reset v12 after a timeout. Requires our waspmoteapi fork.
 */
void onHAIwakeUP_after(void)
{
  if (_boot_version < 'H')
  {
    if (intFlag & RTC_INT)
    {
      RTC.ON();
      if (RTC.alarmTriggered & 2) // Alarm 2
      {
        intFlag &= ~(RTC_INT); RTC.alarmTriggered = 0;
        RTC.disableAlarm2();
        PWR.reboot();
      }
      RTC.OFF();
    }
  }
}
