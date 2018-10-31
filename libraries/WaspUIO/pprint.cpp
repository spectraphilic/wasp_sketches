#include "WaspUIO.h"


const char* WaspUIO::pprint4G(char* dst, size_t size)
{
  // XXX Print some info from the module with _4G.getInfo(...) ??
#if WITH_4G
  if (pin == 0 || pin > 9999)
  {
    strncpy_F(dst, F("disabled, set pin"), size);
  }
  else
  {
    //snprintf_F(dst, size, F("pin=%u"), pin);
    strncpy_F(dst, F("pin=XXXX"), size);
  }
#else
  snprintf_F(dst, size, F("4G not enabled, define WITH_4G TRUE"));
#endif

  return dst;
}


const char* WaspUIO::pprintAction(char* dst, size_t size, uint8_t action, const __FlashStringHelper* name)
{
  uint16_t value;
  uint8_t hours, minutes;
  size_t len;

  value = actions[action];
  if (value) {
    len = strlen(dst);
    snprintf_F(dst + len, size - len, name);
    len = strlen(dst);

    hours = value / 60;
    minutes = value % 60;
    if (hours == 0) {
      snprintf_F(dst + len, size - len, F("=%hhum "), minutes);
    }
    else if (minutes == 0) {
      snprintf_F(dst + len, size - len, F("=%hhuh "), hours);
    }
    else {
      snprintf_F(dst + len, size - len, F("=%hhuh%hhu "), hours, minutes);
    }
  }

  return dst;
}

const char* WaspUIO::pprintActions(char* dst, size_t size)
{
  dst[0] = '\0';
  uint16_t value;
  uint8_t hours, minutes;

  pprintAction(dst, size, RUN_NETWORK, F("Network"));
  pprintAction(dst, size, RUN_BATTERY, F("Battery"));
  pprintAction(dst, size, RUN_GPS, F("GPS"));
  pprintAction(dst, size, RUN_CTD10, F("CTD10"));
  pprintAction(dst, size, RUN_DS2, F("DS2"));
  pprintAction(dst, size, RUN_DS1820, F("DS1820"));
  pprintAction(dst, size, RUN_BME280, F("BME280"));
  pprintAction(dst, size, RUN_MB, F("MB7389"));
  pprintAction(dst, size, RUN_WS100, F("WS100"));

  if (! dst[0]) strncpy_F(dst, F("(none)"), size);

  return dst;
}

const char* WaspUIO::pprintBattery(char* dst, size_t size)
{
  dst[0] = '\0';
  if (batteryType == BATTERY_LITHIUM)
  {
    snprintf_F(dst, size, F("Lithium-ion (%d %%)"), UIO.batteryLevel);
  }
  else
  {
    char aux[10];
    Utils.float2String(UIO.batteryVolts, aux, 2);
    snprintf_F(dst, size, F("Lead-acid (%s volts)"), aux);
  }

  return dst;
}

const char* WaspUIO::pprintBoard(char* dst, size_t size)
{
  dst[0] = '\0';
  if (boardType == BOARD_LEMMING) { strncat_F(dst, F("lemming"), size); }
  else                            { strncat_F(dst, F("none"), size); }

  return dst;
}

const char* WaspUIO::pprintFrames(char* dst, size_t size)
{
  if (strlen(password) == 0)
  {
    snprintf_F(dst, size, F("size=%u encryption=disabled"), frame.getFrameSize());
  }
  else
  {
    snprintf_F(dst, size, F("size=%u encryption=enabled"), frame.getFrameSize());
  }

  return dst;
}

const char* WaspUIO::pprintLog(char* dst, size_t size)
{
  dst[0] = '\0';
  if (flags & FLAG_LOG_USB) strnjoin_F(dst, size, F(", "), F("USB"));
  if (flags & FLAG_LOG_SD)  strnjoin_F(dst, size, F(", "), F("SD"));
  return dst;
}

const char* WaspUIO::pprintSerial(char* dst, size_t size)
{
  uint8_t serial[8];

  if (_boot_version < 'H')
  {
    for (uint8_t i=0; i < 4; i++) { serial[i] = _serial_id[7-i]; }
    sprintf(dst, "%lu", * (uint32_t*) serial);
  }
  else
  {
    for (uint8_t i=0; i < 8; i++) { serial[i] = _serial_id[i]; }
    Utils.hex2str(serial, dst, 8);
  }

  return dst;
}

const char* WaspUIO::pprintXBee(char* dst, size_t size)
{
  char hw[5];
  char sw[9];
  char macH[9];
  char macL[9];
  char name[20];

#if WITH_XBEE
  Utils.hex2str(xbeeDM.hardVersion, hw, 2);
  Utils.hex2str(xbeeDM.softVersion, sw, 4);
  Utils.hex2str(xbeeDM.sourceMacHigh, macH, 4);
  Utils.hex2str(xbeeDM.sourceMacLow, macL, 4);
  strncpy_P(name, xbee.name, sizeof name);
  snprintf_F(dst, size, F("mac=%s%s hw=%s sw=%s network=\"%s\""), macH, macL, hw, sw, name);
#else
  snprintf_F(dst, size, F("XBee not enabled, define WITH_XBEE TRUE"));
#endif

  return dst;
}
