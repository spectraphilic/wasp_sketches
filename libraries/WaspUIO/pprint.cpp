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
    //snprintf_P(dst, size, PSTR("pin=%u"), pin);
    strncpy_F(dst, F("pin=XXXX"), size);
  }

  return dst;
}
#endif


const char* WaspUIO::pprintAction(char* dst, size_t size, uint8_t idx, PGM_P name)
{
  Action action = actions[idx];

  if (action.type != action_disabled)
  {
    size_t len;

    len = strlen(dst); snprintf_P(dst + len, size - len, name);
    len = strlen(dst); snprintf_P(dst + len, size - len, PSTR("="));

    len = strlen(dst);
    if (action.type == action_hours)
    {
      snprintf_P(dst + len, size - len, PSTR("%d:%02d "), action.hour, action.minute);
    }
    else if (action.type == action_minutes)
    {
      snprintf_P(dst + len, size - len, PSTR("%d "), action.minute);
    }
  }

  return dst;
}

const char* WaspUIO::pprintActions(char* dst, size_t size)
{
  dst[0] = '\0';

  for (uint8_t i=0; i < RUN_LEN; i++)
  {
    const char* name = (PGM_P)pgm_read_word(&(run_names[i]));
    pprintAction(dst, size, i, name);
  }

  if (! dst[0]) strncpy_F(dst, F("(none)"), size);

  return dst;
}

const char* WaspUIO::pprintBattery(char* dst, size_t size)
{
  dst[0] = '\0';
  if (batteryType == BATTERY_REG3V3)
  {
    char aux[10];
    Utils.float2String(UIO.batteryVolts, aux, 2);
    snprintf_P(dst, size, PSTR("3V3 regulator (%s volts)"), aux) ;
  }
  else // BATTERY_LITHIUM
  {
    snprintf_P(dst, size, PSTR("Lithium-ion (%d %%)"), UIO.batteryLevel);
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
    snprintf_P(dst, size, PSTR("payload-size=%u frame-size=%u encryption=enabled"), payloadSize, frameSize);
    return dst;
  }
#endif

  snprintf_P(dst, size, PSTR("payload-size=%u frame-size=%u encryption=disabled"), payloadSize, frameSize);
  return dst;
}

#if WITH_IRIDIUM
const char* WaspUIO::pprintIridium(char* dst, size_t size)
{
  snprintf_P(dst, size, PSTR("fw=%s"), iridium_fw);
  return dst;
}
#endif

const char* WaspUIO::pprintLog(char* dst, size_t size)
{
  dst[0] = '\0';
  if (log_usb) strnjoin_F(dst, size, F(", "), F("USB"));
  if (log_sd)  strnjoin_F(dst, size, F(", "), F("SD"));
  return dst;
}

const char* WaspUIO::pprintSerial(char* dst, size_t size)
{
  uint8_t serial[8];

  for (uint8_t i=0; i < 8; i++) { serial[i] = _serial_id[i]; }
  Utils.hex2str(serial, dst, 8);

  return dst;
}

const char* WaspUIO::pprintTime(char* dst, size_t size)
{
  timestamp_t ts;
  const char* day;
  uint32_t epoch = getEpochTime();

  RTC.breakTimeAbsolute(epoch, &ts);
  switch (ts.day)
  {
    case 1: day = DAY_1; break;
    case 2: day = DAY_2; break;
    case 3: day = DAY_3; break;
    case 4: day = DAY_4; break;
    case 5: day = DAY_5; break;
    case 6: day = DAY_6; break;
    case 7: day = DAY_7; break;
    default: day = "???";
  }

  snprintf_P(dst, size, PSTR("%s, %02u/%02u/%02u, %02u:%02u:%02u"),
             day, ts.year, ts.month, ts.date, ts.hour, ts.minute, ts.second);

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
  snprintf_P(dst, size, PSTR("mac=%s%s hw=%s sw=%s network=\"%s\" wait=%d"), macH, macL, hw, sw, name, lan_wait);

  return dst;
}
#endif

#if WITH_LORA
const char* WaspUIO::pprintLora(char* dst, size_t size)
{
  snprintf_P(dst, size, PSTR("addr=%u dst=%u mode=%u wait=%d"), lora_addr, lora_dst, lora_mode, lan_wait);
  return dst;
}
#endif
