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


/**
 * Set time from the network
 * Return: 0=success 1=error
 */

uint8_t WaspUIO::setTimeFromNetwork()
{
  uint8_t err = 1;

  if (networkType == NETWORK_XBEE)
  {
    // Right now it's the Pi who pushes the time to the Mote
    // This may change in the future to make the Mote pull the time from the Pi
    // so it would work like with 4G
    error(F("Setting time from XBee network NOT implemented"));
    return 1;
  }

  else if (networkType == NETWORK_4G)
  {
#if WITH_4G
    err = _4GStart();
    if (err == 0)
    {
      err = _4G.setTimeFrom4G(true);
      if (err == 0)
      {
        loadTime(false);
      }
      _4GStop();
    }
#else
    error(F("4G not enabled, define WITH_4G TRUE"));
#endif
  }

  if (err) { error(F("Failed to set time from network")); }
  else     { debug(F("Success setting time from network")); }

  return err;
}
