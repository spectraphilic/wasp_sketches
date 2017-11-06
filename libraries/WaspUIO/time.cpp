#include "WaspUIO.h"

/**
 * Saves the given time to the hardware clock (RTC). Updates the system time as
 * well.
 */
uint8_t WaspUIO::saveTime(uint8_t year, uint8_t month, uint8_t day,
                          uint8_t hour, uint8_t minute, uint8_t second)
{
  uint8_t rtcON = RTC.isON;
  if (! rtcON) { RTC.ON(); } // RTC ON

  // Save time to RTC
  uint8_t dow = RTC.dow(year, month, day);
  uint8_t err = RTC.setTime(year, month, day, dow, hour, minute, second);
  if (err)
  {
    error(F("setTime: RTC.setTime failed (please double check the syntax)"));
  }
  else
  {
    loadTime();
  }

  if (! rtcON) { RTC.OFF(); } // RTC OFF

  return err;
}


/**
 * Sets the system time from the time in the hardware clock (RTC).
 * Optionally read the temperature as well.
 */
void WaspUIO::loadTime(bool temp)
{
  uint8_t rtcON = RTC.isON;
  if (! rtcON) { RTC.ON(); } // RTC ON

  epochTime = RTC.getEpochTime();
  start = millis() - cr.sleep_time;

  // Read temperature now that RTC is ON (v12 only), it takes less than 2ms
  if (temp && _boot_version < 'H')
  {
    rtc_temp = RTC.getTemperature();
  }

  if (! rtcON) { RTC.OFF(); } // RTC OFF
}


/**
 * getEpochTime
 *
 * Return the system time (seconds since the epoch).
 * Optionally with milliseconds precision.
 */

unsigned long WaspUIO::getEpochTime()
{
  uint32_t diff = (millis() - start) + cr.sleep_time;

  return epochTime + (diff / 1000);
}

unsigned long WaspUIO::getEpochTime(uint16_t &ms)
{
  uint32_t diff = (millis() - start) + cr.sleep_time;

  ms = diff % 1000;
  return epochTime + diff / 1000;
}
