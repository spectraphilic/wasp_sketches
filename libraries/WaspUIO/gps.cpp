#include "WaspUIO.h"


int8_t WaspUIO::gps(bool setTime, bool getPosition)
{
  const __FlashStringHelper * error_msg = NULL;
  uint8_t satellites;

  log_debug("GPS start");
  if (_boot_version >= 'J') { stopSD(); }

  // On
  if (GPS.ON() == 0)
  {
    log_error("GPS.ON() failure");
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
    for (int i=0; i < 3; i++)
    {
      delay(10000); // 10s
      int8_t status = GPS.getPosition();
      if (status == 1)
      {
        satellites = (uint8_t) atoi(GPS.satellites);
        if (satellites > 4)
        {
          break;
        }
      }
      else if (status == -1)
      {
        error_msg = F("GPS.getPosition() No GPS signal");
        goto exit;
      }
      else // if (status == 0)
      {
        error_msg = F("GPS.getPosition() Timeout");
        goto exit;
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
    log_info("GPS Time updated!");
  }

  if (getPosition)
  {
    float lat = GPS.convert2Degrees(GPS.latitude , GPS.NS_indicator);
    float lon = GPS.convert2Degrees(GPS.longitude, GPS.EW_indicator);
    float alt = atof(GPS.altitude);
    float acc = atof(GPS.accuracy);

    // Debug
    char lat_str[15];
    char lon_str[15];
    Utils.float2String(lat, lat_str, 6);
    Utils.float2String(lon, lon_str, 6);
    log_debug("GPS latitude  %s %c => %s", GPS.latitude, GPS.NS_indicator, lat_str);
    log_debug("GPS longitude %s %c => %s", GPS.longitude, GPS.EW_indicator, lon_str);
    log_debug("GPS altitude=%s", GPS.altitude);
    log_debug("GPS satellites=%s accuracy=%s", GPS.satellites, GPS.accuracy);

    // Frames
    ADD_SENSOR(SENSOR_GPS, lat, lon);
    ADD_SENSOR(SENSOR_ALTITUDE, alt)
    ADD_SENSOR(SENSOR_GPS_ACCURACY, satellites, acc);

  }

exit:
  if (error_msg)
  {
    GPS.OFF();
    if (_boot_version >= 'J') { startSD(); }
    cr.log(LOG_ERROR, error_msg);
    return -1;
  }
  return 0;
}
