#include "WaspUIO.h"


#if WITH_4G
const char* WaspUIO::pprint4G(char* dst, size_t size)
{
  // XXX Print some info from the module with _4G.getInfo(...) ??
  if (pin == 0 || pin > 9999)
  {
    strncpy_F(dst, F("disabled, set pin"), size);
  }
  else
  {
    //snprintf_F(dst, size, F("pin=%u"), pin);
    strncpy_F(dst, F("pin=XXXX"), size);
  }

  return dst;
}
#endif


const char* WaspUIO::pprintAction(char* dst, size_t size, uint8_t action, const __FlashStringHelper* name)
{
  uint16_t value = actions[action];

  if (value) {
    size_t len = strlen(dst);
    snprintf_F(dst + len, size - len, name);
    len = strlen(dst);

    uint8_t hours = value / 60;
    uint8_t minutes = value % 60;
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

  for (uint8_t i=0; i < RUN_LEN; i++)
  {
    const char* name = (const char*)pgm_read_word(&(run_names[i]));
    pprintAction(dst, size, i, (__FlashStringHelper*)name);
  }

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
#if WITH_CRYPTO
  if (strlen(password) > 0)
  {
    snprintf_F(dst, size, F("size=%u encryption=enabled"), frame.getFrameSize());
    return dst;
  }
#endif

  snprintf_F(dst, size, F("size=%u encryption=disabled"), frame.getFrameSize());
  return dst;
}

#if WITH_IRIDIUM
const char* WaspUIO::pprintIridium(char* dst, size_t size)
{
  int status;
  char version[10];

  // ON
  status = iridium_start();
  if (status != ISBD_SUCCESS)
  {
    snprintf_F(dst, size, F("error=%d"), status);
  }
  else
  {
    status = iridium.getFirmwareVersion(version, sizeof version);
    if (status != ISBD_SUCCESS)
    {
      snprintf_F(dst, size, F("error=%d"), status);
    }
    else
    {
      snprintf_F(dst, size, F("firmware=%s"), version);
    }
  }

  iridium_stop();

  return dst;
}
#endif

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
    sprintf(dst, "%lu", * (unsigned long*) serial);
  }
  else
  {
    for (uint8_t i=0; i < 8; i++) { serial[i] = _serial_id[i]; }
    Utils.hex2str(serial, dst, 8);
  }

  return dst;
}

#if WITH_XBEE
const char* WaspUIO::pprintXBee(char* dst, size_t size)
{
  char hw[5];
  char sw[9];
  char macH[9];
  char macL[9];
  char name[20];

  Utils.hex2str(xbeeDM.hardVersion, hw, 2);
  Utils.hex2str(xbeeDM.softVersion, sw, 4);
  Utils.hex2str(xbeeDM.sourceMacHigh, macH, 4);
  Utils.hex2str(xbeeDM.sourceMacLow, macL, 4);
  strncpy_P(name, xbee.name, sizeof name);
  snprintf_F(dst, size, F("mac=%s%s hw=%s sw=%s network=\"%s\""), macH, macL, hw, sw, name);

  return dst;
}
#endif
