#include "WaspUIO.h"


int8_t WaspUIO::gps(bool setTime, bool getPosition)
{
  // Check feature availability
  if (! UIO.hasGPS) { return -1; }

  // On
  if (GPS.ON() == 0)
  {
    warn(F("GPS.ON() Error"));
    return -1;
  }

  // Ephemerides
  uint32_t start = millis();
//if (GPS.loadEphems() == 1)
//{
//  debug(F("GPS.loadEphems() Done"));
//}
//else
//{
//  warn(F("GPS.loadEphems() Error"));
//}

  // Connect
  // XXX We could use GPS.check instead, and give control back with CR_DELAY,
  // problem is when we sleep (cr) the gps is powered off (to verify).
  if (GPS.waitForSignal(150) == false) // 150s = 2m30s
  {
    warn(F("GPS.waitForSignal() Timeout"));
    GPS.OFF();
    return -1;
  }

  // Ephemerides
  if (GPS.saveEphems() == 1)
  {
    debug(F("GPS.saveEphems() Done"));
  }
  else
  {
    warn(F("GPS.saveEphems() Error"));
  }
  uint32_t time = millis() - start;

  // Position
  if (getPosition)
  {
    if (GPS.getPosition() != 1)
    {
      warn(F("GPS.getPosition() Error"));
      GPS.OFF();
      return -1;
    }
  }

  // Time
  // XXX could optimize, as part of the work in setTimeFromGPS is already done
  // in getPosition above.
  uint32_t before, after;
  if (setTime)
  {
    before = UIO.getEpochTime();
    GPS.setTimeFromGPS(); // Save time to RTC
    UIO.loadTime(); // Set system time
    after = UIO.getEpochTime();
    info(F("GPS: Time updated"));
  }

  GPS.OFF();

  if (getPosition)
  {
    debug(F("GPS: Position latitude=%s %c"), GPS.latitude, GPS.NS_indicator);
    debug(F("GPS: Position longitude=%s %c"), GPS.longitude, GPS.EW_indicator);
    debug(F("GPS: Position altitude=%s course=%s speed=%s"), GPS.altitude, GPS.course, GPS.speed);
    ADD_SENSOR(SENSOR_GPS, GPS.convert2Degrees(GPS.latitude , GPS.NS_indicator),
                           GPS.convert2Degrees(GPS.longitude, GPS.EW_indicator));
    //ADD_SENSOR(SENSOR_ALTITUDE, GPS.altitude);
    //ADD_SENSOR(SENSOR_SPEED, GPS.speed);
    //ADD_SENSOR(SENSOR_COURSE, GPS.course);
  }

  if (setTime)
  {
    uint32_t skew = (after > before) ? (after - before): (before - after);
    info(F("GPS: Success, time updated (time=%lu skew=%lu)"), time, skew);
    // Frames
    //ADD_SENSOR(SENSOR_GPS_STATS, time, skew);
  }

  // OK
  return 0;
}
