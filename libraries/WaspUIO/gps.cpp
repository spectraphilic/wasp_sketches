#include "WaspUIO.h"


int8_t WaspUIO::gps(bool setTime, bool getPosition)
{
  const __FlashStringHelper * err = NULL;

  debug(F("GPS start"));
  if (_boot_version >= 'J') { stopSD(); }

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
  if (_boot_version >= 'J') { startSD(); }

  if (setTime)
  {
    info(F("GPS Time updated!"));
  }

  if (getPosition)
  {
    float lat = GPS.convert2Degrees(GPS.latitude , GPS.NS_indicator);
    float lon = GPS.convert2Degrees(GPS.longitude, GPS.EW_indicator);

    // Debug
    char lat_str[15];
    char lon_str[15];
    Utils.float2String(lat, lat_str, 6);
    Utils.float2String(lon, lon_str, 6);
    debug(F("GPS latitude  %s %c => %s"), GPS.latitude, GPS.NS_indicator, lat_str);
    debug(F("GPS longitude %s %c => %s"), GPS.longitude, GPS.EW_indicator, lon_str);
    debug(F("GPS altitude=%s course=%s speed=%s"), GPS.altitude, GPS.course, GPS.speed);

    // Frame
    ADD_SENSOR(SENSOR_GPS, lat, lon);
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
    if (_boot_version >= 'J') { startSD(); }
    error(err);
    return -1;
  }
  return 0;
}
