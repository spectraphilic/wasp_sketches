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
  if (err) {
    log_warn("saveTime: RTC.setTime(%lu) failed", time);
  }

  if (! rtcON) { RTC.OFF(); } // RTC OFF
  return err;
}


uint8_t WaspUIO::saveTimeToSD()
{
    SdFile file;

    if (sd_open(timeFilename, file, O_WRITE | O_CREAT | O_TRUNC | O_SYNC)) {
        log_warn("sd_open(TIME.TXT) failure  flag=%u %d", SD.flag, SD.card.errorCode());
        return 1;
    }

    char buffer[11];
    uint32_t time = getEpochTime();
    cr_snprintf(buffer, 11, "%lu", time);
    file.write(buffer); // TODO Check error code
    file.close();

    return 0;
}


/**
 * Sets the given time. Stores it in RTC and SD, then updates the system time.
 * Returns 1 if error, 0 if success.
 */
uint8_t WaspUIO::setTime(uint8_t year, uint8_t month, uint8_t day,
                         uint8_t hour, uint8_t minute, uint8_t second)
{
  _epoch_millis = millis();
  _epoch = RTC.getEpochTime(year, month, day, hour, minute, second);
  return saveTime();
}

uint8_t WaspUIO::setTime(uint32_t time)
{
  _epoch_millis = millis();
  _epoch = time;
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
  _epoch = RTC.getEpochTime();
  _epoch_millis = millis();

  // If it's an old time, read it from the SD
  if (_epoch < 1672527600) // 2023-01-01 A date in the past
  {
    log_warn("Wrong time detected");
    if (sd_open(timeFilename, file, O_READ))
    {
       log_warn("Opening TIME.TXT failed");
    }
    else
    {
      int size = file.read(SD.buffer, 10);
      file.close();
      if (size != 10)
      {
        log_warn("Reading TIME.TXT failed");
      }
      else
      {
        _epoch = strtoul(SD.buffer, NULL, 10);
        _epoch_millis = millis();
        saveTime();
        log_info("Time loaded from TIME.TXT");
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
  uint32_t diff = (millis() - _epoch_millis);

  return _epoch + (diff / 1000);
}

unsigned long WaspUIO::getEpochTime(uint16_t &ms)
{
  uint32_t diff = (millis() - _epoch_millis);

  ms = diff % 1000;
  return _epoch + diff / 1000;
}


/**
 * Set time from the network
 * Return: 0=success 1=error
 */

uint8_t WaspUIO::setTimeFromNetwork()
{
  uint8_t err = 1;

#if WITH_4G
  if (wan_type == WAN_4G)
  {
    err = _4GStart();
    if (err == 0)
    {
      err = setTimeFrom4G();
      _4GStop();
    }
    return err;
  }
#endif

  log_error("Setting time from network only supported in 4G networks");
  return err;
}
