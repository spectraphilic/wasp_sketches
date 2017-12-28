#include "WaspUIO.h"


/**
 * Set RTC time from GPS
 *
 * TODO Use ephemiris, apparently this is required to improve power.
 */

CR_TASK(taskGps)
{
  uint32_t before, after;
  uint32_t start, time;

  // On
  if (GPS.ON() == 0)
  {
    warn(F("GPS: GPS.ON() failed"));
    return CR_TASK_ERROR;
  }

  debug(F("GPS: Start"));
  start = millis();

  // Ephemerides
  if (UIO.hasSD)
  {
    if (GPS.loadEphems() == 1)
    {
      debug(F("GPS: Ephemerides loaded"));
    }
    else
    {
      warn(F("GPS: Ephemerides loading failed"));
    }
  }

  // XXX We could use GPS.check instead, and give control back with CR_DELAY,
  // problem is when we sleep (cr) the gps is powered off (to verify).
  if (GPS.waitForSignal(150) == false) // 150s = 2m30s
  {
    warn(F("GPS: Timeout"));
    GPS.OFF();
    return CR_TASK_ERROR;
  }

  // Ephemerides
  if (UIO.hasSD)
  {
    if (GPS.saveEphems() == 1)
    {
      debug(F("GPS: Ephemerides saved"));
    }
    else
    {
      warn(F("GPS: Ephemerides saving failed"));
    }
  }
  time = millis() - start;

  // Position
  if (GPS.getPosition() != 1)
  {
    warn(F("GPS: getPosition failed"));
    GPS.OFF();
    return CR_TASK_ERROR;
  }

  // Time
  // XXX could optimize, as part of the work in setTimeFromGPS is already done
  // in getPosition above.
  before = UIO.getEpochTime();
  GPS.setTimeFromGPS(); // Save time to RTC
  UIO.loadTime(); // Set system time
  after = UIO.getEpochTime();
  GPS.OFF();

  // Frames
  uint32_t skew = (after > before) ? (after - before): (before - after);
  info(F("GPS: Success, time updated (time=%lu skew=%lu)"), time, skew);
  debug(F("GPS: Position latitude=%s %c"), GPS.latitude, GPS.NS_indicator);
  debug(F("GPS: Position longitude=%s %c"), GPS.longitude, GPS.EW_indicator);
  debug(F("GPS: Position altitude=%s course=%s speed=%s"), GPS.altitude, GPS.course, GPS.speed);
  //ADD_SENSOR(SENSOR_GPS_STATS, time, skew);
  ADD_SENSOR(SENSOR_GPS, GPS.convert2Degrees(GPS.latitude , GPS.NS_indicator),
                         GPS.convert2Degrees(GPS.longitude, GPS.EW_indicator));
  //ADD_SENSOR(SENSOR_ALTITUDE, GPS.altitude);
  //ADD_SENSOR(SENSOR_SPEED, GPS.speed);
  //ADD_SENSOR(SENSOR_COURSE, GPS.course);

  // Off
  return CR_TASK_STOP;
}
