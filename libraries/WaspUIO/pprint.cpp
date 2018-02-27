#include "WaspUIO.h"


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

const char* WaspUIO::pprintBattery(char* dst, size_t size)
{
  dst[0] = '\0';
  if      (batteryType == 1) strncpy_F(dst, F("Lithium-ion"), size);
  else if (batteryType == 2) strncpy_F(dst, F("Lead acid"), size);
  return dst;
}

const char* WaspUIO::pprintLog(char* dst, size_t size)
{
  dst[0] = '\0';
  if (flags & FLAG_LOG_USB) strnjoin_F(dst, size, F(", "), F("USB"));
  if (flags & FLAG_LOG_SD)  strnjoin_F(dst, size, F(", "), F("SD"));
  return dst;
}

const char* WaspUIO::pprintNetwork(char* dst, size_t size)
{
  if (actions[RUN_NETWORK] == 0) strncpy_F(dst, F("Disabled"), size);
  else                           strncpy_P(dst, network.name, size);
  return dst;
}

const char* WaspUIO::pprintActions(char* dst, size_t size)
{
  dst[0] = '\0';
  uint8_t value;

  value = actions[RUN_NETWORK];
  if (value) strnjoin_F(dst, size, F(", "), F("Network (%d)"), value);
#ifdef USE_AGR
  value = actions[RUN_SENSIRION];
  if (value) strnjoin_F(dst, size, F(", "), F("Sensirion (%d)"), value);
  value = actions[RUN_PRESSURE];
  if (value) strnjoin_F(dst, size, F(", "), F("Pressure (%d)"), value);
  value = actions[RUN_LEAFWETNESS];
  if (value) strnjoin_F(dst, size, F(", "), F("Leaf Wetness (%d)"), value);
#endif
#ifdef USE_SDI
  value = actions[RUN_CTD10];
  if (value) strnjoin_F(dst, size, F(", "), F("CTD-10 (%d)"), value);
  value = actions[RUN_DS2];
  if (value) strnjoin_F(dst, size, F(", "), F("DS-2 (%d)"), value);
#endif
#ifdef USE_I2C
  value = actions[RUN_DS1820];
  if (value) strnjoin_F(dst, size, F(", "), F("DS1820 (%d)"), value);
  value = actions[RUN_BME280];
  if (value) strnjoin_F(dst, size, F(", "), F("BME-280 (%d)"), value);
  value = actions[RUN_MB];
  if (value) strnjoin_F(dst, size, F(", "), F("MB7389 (%d)"), value);
#endif
  if (! dst[0]) strncpy_F(dst, F("(none)"), size);
  return dst;
}
