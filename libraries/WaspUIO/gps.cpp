#include "WaspUIO.h"


int8_t WaspUIO::gps(bool setTime, bool getPosition)
{
  const __FlashStringHelper * err = NULL;

  debug(F("GPS start"));

  // On
  if (GPS.ON() == 0)
  {
    error(F("GPS.ON() failure"));
    return -1;
  }

  // Connect
  if (GPS.waitForSignal(150) == false) // 150s = 2m30s
  {
    err = F("GPS Timeout");
    goto exit;
  }

  // Position
  if (getPosition && GPS.getPosition() != 1)
  {
    err = F("GPS.getPosition() Error");
    goto exit;
  }

  // Time
  // TODO optimize, part of the work in setTimeFromGPS is already done in
  // getPosition above
  if (setTime)
  {
    GPS.setTimeFromGPS(); // Save time to RTC
    UIO.loadTime(); // Set system time
  }

  GPS.OFF();

  if (setTime)
  {
    info(F("GPS Time updated!"));
  }

  if (getPosition)
  {
    debug(F("GPS latitude=%s %c"), GPS.latitude, GPS.NS_indicator);
    debug(F("GPS longitude=%s %c"), GPS.longitude, GPS.EW_indicator);
    debug(F("GPS altitude=%s course=%s speed=%s"), GPS.altitude, GPS.course, GPS.speed);
    ADD_SENSOR(SENSOR_GPS, GPS.convert2Degrees(GPS.latitude , GPS.NS_indicator),
                           GPS.convert2Degrees(GPS.longitude, GPS.EW_indicator));
    if (GPS.altitude)
    {
      ADD_SENSOR(SENSOR_ALTITUDE, atof(GPS.altitude));
    }
    //ADD_SENSOR(SENSOR_SPEED, atof(GPS.speed));
    //ADD_SENSOR(SENSOR_COURSE, atof(GPS.course));
  }

exit:
  if (err)
  {
    GPS.OFF();
    error(err, xbeeDM.error_AT);
    return -1;
  }
  return 0;
}
