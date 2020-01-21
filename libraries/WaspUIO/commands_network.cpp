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
  UIO.loraInit();

  // uint8_t unless specified otherwise
  cr.println(F("Mode BW=%u CR=%u SF=%u"), sx1272._bandwidth, sx1272._codingRate, sx1272._spreadingFactor);
  cr.println(F("Header = %u"), sx1272._header);
  cr.println(F("CRC = %u"), sx1272._CRC);
  cr.println(F("Channel = %lX"), sx1272._channel); // uint32_t
  cr.println(F("Power = %u"), sx1272._power);
  cr.println(F("Preamble length = %u"), sx1272._preamblelength); // uint16_t
  cr.println(F("Payload length = %u"), sx1272._payloadlength);
  cr.println(F("Node address = %u"), sx1272._nodeAddress);
  cr.println(F("Max current = %u mA"), sx1272._maxCurrent); // 0x00 to 0x1B (5 to 240 mA)
  cr.println(F("Temp = %d"), sx1272._temp); // int

//sx1272.setTimeout();
//cr.println(F("Send time = %u ms"), sx1272._sendTime); // uint16_t

  return cmd_ok;
}
#endif


/**
 * Network testing: ping
 */

COMMAND(cmdPing)
{
  /*
   * If the ping command is local, then test the network. How depends on the
   * network used; for local networks send the "ping" command to the remote
   * host.
   */
#if WITH_4G
  if (UIO.wan_type == WAN_4G)
  {
    UIO._4GPing();
    return cmd_quiet;
  }
#endif
#if WITH_IRIDIUM
  if (UIO.wan_type == WAN_IRIDIUM)
  {
    UIO.iridium_ping();
    return cmd_quiet;
  }
#endif
#if WITH_XBEE
  if (UIO.lan_type == LAN_XBEE)
  {
    if (UIO.xbeeSend(UIO.xbee.rx_address, "ping"))
      return cmd_error;
    info(F("RSSI(dBm) = %d"), UIO.rssi);
    return cmd_quiet;
  }
#endif
#if WITH_LORA
  if (UIO.lan_type == LAN_LORA)
  {
    if (UIO.loraSend(1, "ping", true))
      return cmd_error;
    info(F("RSSI(dBm) = %d SNR = %d"), UIO.rssi, UIO.snr);
    return cmd_quiet;
  }
#endif

  return cmd_error;
}
