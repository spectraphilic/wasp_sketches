#include "WaspUIO.h"


int8_t WaspUIO::gps(bool setTime, bool getPosition)
{
  const __FlashStringHelper * error_msg = NULL;
  int satellites;

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
    error_msg = F("GPS Timeout");
    goto exit;
  }

  // Position
  if (getPosition)
  {
    // Try twice to get enough satellites (4), wait 10s before each try
    for (int i=0; i < 2; i++)
    {
      delay(10000); // 10s
      if (GPS.getPosition() != 1)
      {
        error_msg = F("GPS.getPosition() Error");
        goto exit;
      }
      satellites = atoi(GPS.satellites);
      if (satellites >= 4)
      {
        break;
      }
    }
  }

  // Time
  // TODO optimize, part of the work in setTimeFromGPS is already done in
  // getPosition above
  if (setTime)
  {
    GPS.setTimeFromGPS(); // Save time to RTC
  }

  GPS.OFF();
  if (_boot_version >= 'J') { startSD(); }
  // Set system time. Do this here because we need the SD
  if (setTime)
  {
    UIO.loadTime();
  }

  if (setTime)
  {
    info(F("GPS Time updated!"));
  }

  if (getPosition)
  {
    if (satellites < 4)
    {
      warn(F("Only found %d satellites, give up"), satellites);
    }
    else
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
  }

exit:
  if (error_msg)
  {
    GPS.OFF();
    if (_boot_version >= 'J') { startSD(); }
    error(error_msg);
    return -1;
  }
  return 0;
}
