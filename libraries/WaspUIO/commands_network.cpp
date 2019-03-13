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
  uint8_t err = UIO._4GGPS();
  if (err)
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

/* Test 4G data connection */
COMMAND(cmd4G_Test)
{
  uint8_t err = UIO._4GStart();
  if (err)
  {
    return cmd_error;
  }

  UIO._4GStop();
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
  cr.println(F("4G ping not implemented"));
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
