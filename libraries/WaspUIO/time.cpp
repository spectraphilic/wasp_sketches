#include "WaspUIO.h"


/**
 * Saves the system time to the hardware clock (RTC).
 * Returns 1 if error, 0 if success.
 */

uint8_t WaspUIO::saveTime()
{
  timestamp_t ts;
  uint32_t time = getEpochTime();
  RTC.breakTimeAbsolute(time, &ts);

  uint8_t rtcON = RTC.isON;
  if (! rtcON) { RTC.ON(); } // RTC ON

  // Save time to RTC
  uint8_t err = RTC.setTime(ts.year, ts.month, ts.date, ts.day, ts.hour, ts.minute, ts.second);
  if (err)
  {
    warn(F("saveTime: RTC.setTime(%lu) failed"), epochTime);
  }

  if (! rtcON) { RTC.OFF(); } // RTC OFF
  return err;
}


uint8_t WaspUIO::saveTimeToSD()
{
  SdFile file;

  if (sd_open(timeFilename, file, O_WRITE | O_CREAT | O_TRUNC))
  {
    warn(F("saveTimeToSD: Opening TIME.TXT failed"));
    return 1;
  }
  else
  {
    char buffer[11];
    uint32_t time = getEpochTime();
    snprintf_F(buffer, 11, F("%lu"), time);
    file.write(buffer);
    file.close();
  }

  return 0;
}


/**
 * Sets the given time. Stores it in RTC and SD, then updates the system time.
 * Returns 1 if error, 0 if success.
 */
uint8_t WaspUIO::setTime(uint8_t year, uint8_t month, uint8_t day,
                         uint8_t hour, uint8_t minute, uint8_t second)
{
  epochTime = RTC.getEpochTime(year, month, day, hour, minute, second);
  start = millis() - cr.sleep_time;
  return saveTime();
}

uint8_t WaspUIO::setTime(uint32_t time)
{
  epochTime = time;
  start = millis() - cr.sleep_time;
  return saveTime();
}


/**
 * Sets the system time from the time in the hardware clock (RTC).
 */
void WaspUIO::loadTime()
{
  uint8_t rtcON;
  SdFile file;

  // Read time from RTC
  rtcON = RTC.isON;
  if (! rtcON) { RTC.ON(); } // RTC ON
  epochTime = RTC.getEpochTime();
  start = millis() - cr.sleep_time;

  // If it's an old time, read it from the SD
  if (epochTime < 1541030400) // 2018-11-01 A date in the past
  {
    startSD();
    warn(F("Wrong time detected"));
    if (sd_open(timeFilename, file, O_READ))
    {
       warn(F("Opening TIME.TXT failed"));
    }
    else
    {
      int size = file.read(SD.buffer, 10);
      file.close();
      if (size != 10)
      {
        warn(F("Reading TIME.TXT failed"));
      }
      else
      {
        epochTime = strtoul(SD.buffer, NULL, 10);
        start = millis() - cr.sleep_time;
	saveTime();
        info(F("Time loaded from TIME.TXT"));
      }
    }
  }

  // Ok
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
        loadTime();
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
