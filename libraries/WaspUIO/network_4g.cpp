#include "WaspUIO.h"


#if WITH_4G
const char _4G_GPRS [] PROGMEM = "GPRS";
const char _4G_EGPRS [] PROGMEM = "EGPRS";
const char _4G_WCDMA [] PROGMEM = "WCDMA";
const char _4G_HSDPA [] PROGMEM = "HSDPA";
const char _4G_LTE [] PROGMEM = "LTE";
const char _4G_UNKNOWN [] PROGMEM = "UNKNOWN";

const char* const _4G_networks[] PROGMEM = {
  _4G_GPRS,
  _4G_EGPRS,
  _4G_WCDMA,
  _4G_HSDPA,
  _4G_LTE,
  _4G_UNKNOWN,
};


void WaspUIO::_4GInit()
{
}

uint8_t WaspUIO::_4GStart()
{
  uint8_t err, status;
  char pin_str[5];

  // Check pin number
  if (pin == 0 || pin > 9999)
  {
    error(F("Bad pin number (%u), set pin in the menu"), pin);
    return 1;
  }
  snprintf(pin_str, sizeof pin_str, "%04d", pin);

  // Switch on (11s)
  debug(F("4G Switch on"));
  err = _4G.ON();
  if (err)
  {
    error(F("_4G.ON error=%d %d"), err, _4G._errorCode);
    return 1;
  }

  // Enter PIN (0.2s)
  debug(F("4G Enter PIN"));
  status = _4G.checkPIN();
  if (status == 0)
  {
    debug(F("PIN READY"));
  }
  else if (status == 1)
  {
    err = _4G.enterPIN(pin_str);
    if (err)
    {
      pin = 0; updateEEPROM(EEPROM_UIO_PIN, pin); // Reset pin to avoid trying again
      error(F("_4G.enterPIN(%s) error=%d %d"), pin_str, err, _4G._errorCode);
    }
  }
  else
  {
    error(F("unexpected SIM status=%%hhu"), status);
    err = 1;
  }

  // Check data connection: usually ~11s sometimes close to 120s (a 2nd call
  // would take 0.13s)
  if (err == 0)
  {
    debug(F("4G Check data connection"));
    err = _4G.checkDataConnection(120);
    if (err)
    {
      error(F("_4G.checkDataConnection error=%d %d"), err, _4G._errorCode);
    }
    else
    {
      debug(F("4G Data connection OK"));
    }
  }

  // Switch off if error
  if (err)
  {
    _4G.OFF();
  }

  return err;
}

uint8_t WaspUIO::_4GStop()
{
  debug(F("4G Switching off"));
  uint8_t status = _4G.OFF();
  if (status == 0)
  {
     error(F("_4G.OFF() timeout"));
  }
  return 0;
}


/* Test 4G data connection */
int WaspUIO::_4GPing()
{
  uint8_t err = UIO._4GStart();
  if (err)
  {
    return 1;
  }

  // Network type
  err = _4G.getNetworkType();
  if (err == 0 && _4G._networkType < sizeof(_4G_networks) / sizeof(const char*))
  {
    char name[10];
    strcpy_P(name, (char*)pgm_read_word(&(_4G_networks[_4G._networkType])));
    cr.println(F("Network: %s"), name);
  }
  else
  {
    cr.println(F("Network: ERROR"));
  }

  // Operator
  char operator_name[20] = {0};
  err = _4G.getOperator(operator_name);
  if (err == 0)
  {
    cr.println(F("Operator: %s"), operator_name);
  }
  else
  {
    cr.println(F("Operator: ERROR"));
  }

  // RSSI
  err = _4G.getRSSI();
  if (err == 0)
  {
    cr.println(F("RSSI: %d dBm"), _4G._rssi);
  }
  else
  {
    cr.println(F("RSSI: ERROR"));
  }

  UIO._4GStop();
  return 0;
}


uint8_t WaspUIO::setTimeFrom4G(const char *value)
{
  int8_t answer;
  char format[60];
  char command_buffer[50];
  uint8_t year, month, day;
  uint8_t hour, minute, second;
  bool RTC_status;
  int8_t timezone;

  // AT+CCLK?\r
  strcpy_P(command_buffer, (char*)pgm_read_word(&(table_4G[25])));

  // XXX The option to pass a value is only for testing purposes
  if (value == NULL)
  {
    // send command
    answer = _4G.sendCommand(command_buffer, "\"", LE910_ERROR, 2000);
    if (answer != 1)
    {
      return 1;
    }
  
    _4G.waitFor("\"", 2000);
  }
  else
  {
    strcpy((char*)_4G._buffer, value);
  }

  // format <-- "%2hhu%*c%2hhu%*c%2hhu%*c%2hhu%*c%2hhu%*c%2hhu%hhd\""
  // e.g. 02/09/07,22:30:00+08
  // timezone is in quarters, for example +08 (CEST) is +0200
  // 19/04/11,00:00:00+08
  strcpy_P(format, (char*)pgm_read_word(&(table_4G[35])));

  debug(F("4G time: %s"), _4G._buffer);
  sscanf((char*)_4G._buffer, format, &year, &month, &day, &hour, &minute, &second, &timezone);

  // Get current state of RTC power mode
  RTC_status = RTC.isON;
  if (! RTC_status) { RTC.ON(); }

  // Convert to UTC
  uint32_t epoch = RTC.getEpochTime(year, month, day, hour, minute, second);

  if ((epoch / 3600) % 24 == 0) // XXX Skip at midnight
  {
    warn(F("Skip this time because we know it doesn't work in Svalbard"));
    return 0;
  }

  // Set time
  epoch = epoch - (timezone * 15 * 60);
  UIO.setTime(epoch);

  if (! RTC_status) { RTC.OFF(); }

  return 0;
}


uint8_t WaspUIO::_4GGPS()
{
  uint8_t status;

  status = _4GStart();
  if (status != 0)
  {
    return status;
  }

  // GPS modes:
  // - GPS_AUTONOMOUS (slowest, does not require 4G network)
  // - GPS_MS_BASED (faster than autonomous, but requires 4G network)
  // - GPS_MS_ASSISTED (fastest, but less accurate)
  // Reset modes:
  // 1: Coldstart
  // 2: Warmstart
  // 3: Hotstart (default)
  status = _4G.gpsStart(Wasp4G::GPS_MS_BASED, 3);
  if (status != 0)
  {
    error(F("4G gpsStart status=%d error=%u"), status, _4G._errorCode);
    goto exit;
  }

  status = _4G.waitForSignal();
  if (status != 0)
  {
    error(F("4G waitForSignal status=%d error=%u"), status, _4G._errorCode);
  }
  else
  {
    float lat  = _4G.convert2Degrees(_4G._latitude, _4G._latitudeNS);
    float lon = _4G.convert2Degrees(_4G._longitude, _4G._longitudeEW);

    // Debug
    char lat_str[15];
    char lon_str[15];
    char alt_str[15];
    Utils.float2String(lat, lat_str, 6);
    Utils.float2String(lon, lon_str, 6);
    debug(F("GPS latitude  %s %c => %s"), _4G._latitude, _4G._latitudeNS, lat_str);
    debug(F("GPS longitude %s %c => %s"), _4G._longitude, _4G._longitudeEW, lon_str);
    debug(F("GPS altitude=%s course=%s speed=%s"),
      Utils.float2String(_4G._altitude, alt_str, 6),
      _4G._courseOG,
      _4G._speedOG
    );

    // Frame
    ADD_SENSOR(SENSOR_GPS, lat, lon);
    if (_4G._altitude)
    {
      ADD_SENSOR(SENSOR_ALTITUDE, _4G._altitude);
    }
    //ADD_SENSOR(SENSOR_SPEED, _4G._speedOG);
    //ADD_SENSOR(SENSOR_COURSE, atof(_4G.courseOG));
  }

  status = _4G.gpsStop();
  if (status != 0)
  {
    error(F("4G gpsStop status=%d error=%u"), status, _4G._errorCode);
  }

exit:
  _4GStop();
  return status;
}
#endif
