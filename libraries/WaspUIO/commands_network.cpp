#include "WaspUIO.h"


#if WITH_4G
/**
 * Set APN (Access Point Name) for 4G module
 */
COMMAND(cmd4G_APN)
{
  char apn[30];

  // Check input
  int n = sscanf(str, "%29s", apn);
  if (n == -1) // Suprisingly it returns -1 instead of 0
  {
    _4G.show_APN();
  }
  else if (n == 1)
  {
    UIO.writeEEPROM(EEPROM_UIO_APN, apn, sizeof apn);
    _4G.set_APN(apn);
  }
  else
  {
    return cmd_bad_input;
  }

  return cmd_ok;
}

COMMAND(cmd4G_GPS)
{
  if (UIO.hasGPS & GPS_4G == 0) { return cmd_unavailable; }

  if (UIO._4GGPS() != 0)
  {
    return cmd_error;
  }

  return cmd_quiet;
}

COMMAND(cmd4G_Pin)
{
  unsigned int pin;

  int n = sscanf(str, "%u", &pin);
  if (n == -1)
  {
    //cr.println(F("%u"), UIO.pin);
  }
  else if (n == 1)
  {
    if (pin > 9999) { return cmd_bad_input; }
    if (! UIO.updateEEPROM(EEPROM_UIO_PIN, pin)) { return cmd_error; }
    UIO.pin = pin;
  }
  else
  {
    return cmd_bad_input;
  }

  return cmd_ok;
}
#endif


#if WITH_LORA
/**
 * Lora network
 */
COMMAND(cmdLora)
{
  loraInit();

  // uint8_t 0x00 to 0x1B (5 to 240 mA)
  cr.println(F("Max current = %u"), sx1272._maxCurrent);

  cr.println(F("Payload length"), sx1272._payloadlength);

  // uint16_t
  cr.println(F("Preamble length = %u"), sx1272._preamblelength);

  return cmd_ok;
}

COMMAND(cmdLoraAddress)
{
  uint8_t address;

  int n = sscanf(str, "%hhu", &address);
  if (n == -1)
  {
    cr.println(F("%u"), UIO.lora_address);
  }
  else if (n == 1)
  {
    if (! (1 <= address <= 255)) { return cmd_bad_input; }
    if (! UIO.updateEEPROM(EEPROM_UIO_LORA_ADDRESS, address)) { return cmd_error; }
    UIO.lora_address = address;
  }
  else
  {
    return cmd_bad_input;
  }

  return cmd_ok;
}
#endif



/**
 * Choose network type
 */
COMMAND(cmdNetwork)
{
  uint8_t value;

  // Check input
  if (sscanf(str, "%hhu", &value) != 1) { return cmd_bad_input; }
  if (value >= NETWORK_LEN) { return cmd_bad_input; }

  // Do
  if (! UIO.updateEEPROM(EEPROM_UIO_NETWORK_TYPE, value)) { return cmd_error; }
  UIO.networkType = (network_type_t) value;

  // Action
  UIO.setFrameSize();
  UIO.networkInit();

  return cmd_ok;
}

/**
 * Send a test message
 */

COMMAND(cmdPing)
{
#if WITH_XBEE
  int rssi;
  UIO.xbee_ping(rssi);
#endif
#if WITH_4G
  UIO._4GPing();
#endif
#if WITH_IRIDIUM
  UIO.iridium_ping();
#endif

  return cmd_quiet;
}


#if WITH_XBEE
/**
 * Choose Xbee network
 */
COMMAND(cmdXBee)
{
  uint8_t value;

  // Check input
  if (sscanf(str, "%hhu", &value) != 1) { return cmd_bad_input; }
  if (value >= xbee_len) { return cmd_bad_input; }

  // Do
  memcpy_P(&UIO.xbee, &xbees[value], sizeof UIO.xbee);
  if (! UIO.updateEEPROM(EEPROM_UIO_XBEE, UIO.xbee.panid[0]) ||
      ! UIO.updateEEPROM(EEPROM_UIO_XBEE+1, UIO.xbee.panid[1]))
  {
    return cmd_error;
  }
  UIO.xbeeInit();

  return cmd_ok;
}
#endif
