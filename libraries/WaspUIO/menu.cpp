#include "WaspUIO.h"


void WaspUIO::menu()
{
  char buffer[150];
  size_t size = sizeof(buffer);

  RTC.ON();

  // Go interactive or not
  if (input(buffer, sizeof(buffer), F("Press Enter to start interactive mode. Wait 2 seconds to skip:"), 2000) == NULL)
  {
    goto exit;
  }

  do {
    // Menu
    input(buffer, size, F("==> Type 'help' to see the list of commands, 'exit' to leave"), 0);
    int8_t value = exeCommand(buffer);
    if (value == -1)
    {
      cr.print(F("Exit"));
      goto exit;
    }
  } while (true);

exit:
  cr.print();
  RTC.OFF();
}

const char* WaspUIO::menuFormatBattery(char* dst, size_t size)
{
  dst[0] = '\0';
  if      (batteryType == 1) strncpy_F(dst, F("Lithium-ion"), size);
  else if (batteryType == 2) strncpy_F(dst, F("Lead acid"), size);
  return dst;
}

const char* WaspUIO::menuFormatLog(char* dst, size_t size)
{
  dst[0] = '\0';
  if (flags & FLAG_LOG_USB) strnjoin_F(dst, size, F(", "), F("USB"));
  if (flags & FLAG_LOG_SD)  strnjoin_F(dst, size, F(", "), F("SD"));
  return dst;
}

const char* WaspUIO::menuFormatNetwork(char* dst, size_t size)
{
  if (flags & FLAG_NETWORK) strncpy(dst, network.name, size);
  else                      strncpy_F(dst, F("Disabled"), size);
  return dst;
}

const char* WaspUIO::menuFormatActions(char* dst, size_t size)
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

/*
 * Menu: network
 */

void WaspUIO::setNetwork(network_t value)
{
  // Check input parameter is valid
  if (value < NETWORK_FINSE || NETWORK_PI_CS < value)
  {
      cr.print(F("ERROR No network configuration"));
      return;
  }

  // Enable network
  UIO.flags |= FLAG_NETWORK;
  updateEEPROM(EEPROM_UIO_FLAGS, UIO.flags);

  // Save panID to EEPROM
  memcpy_P(&network, &networks[value], sizeof network);
  if (
    updateEEPROM(EEPROM_UIO_NETWORK, network.panid[0]) &&
    updateEEPROM(EEPROM_UIO_NETWORK+1, network.panid[1])
  )
  {
    cr.print(F("INFO Network id saved to EEPROM"));
  }
  else
  {
    cr.print(F("ERROR Saving network id to EEPROM failed"));
  }

  initNet();
}


void WaspUIO::initNet()
{
  uint8_t addressing = UNICAST_64B;
  network_t value = (network_t) network.panid[1]; // panid low byte
  const __FlashStringHelper * error = NULL;

  // Addressing
  memcpy_P(&network, &networks[value], sizeof network);
  cr.print(F("Configuring network: %s"), network.name);
  if (strcmp(network.rx_address, "000000000000FFFF") == 0)
  {
    addressing = BROADCAST_MODE;
  }

  // This is common to all networks, for now
  frame.setID((char*) "");
  //Utils.setAuthKey(key_access);

  // Set Frame size. Will be 73 bytes for XBeeDM-pro S1
  // linkEncryption = DISABLED (not supported by DIGIMESH, apparently)
  // AESEncryption = DISABLED
  uint16_t size = frame.getMaxSizeForXBee(DIGIMESH, addressing, DISABLED, DISABLED);
  frame.setFrameSize(size);
  cr.print(F("Frame size is %d"), frame.getFrameSize());

  // init XBee
  xbeeDM.ON();
  delay(50);

  readOwnMAC();

  // XXX Reduce the number of retries to reduce the time it is lost in send
  // failures (default is 3).
  // 3 retries ~ 5s ; 2 retries ~ 3.5 s ; 1 retry ~ 2.4s
  xbeeDM.setSendingRetries(2);

  // Set channel, check AT commmand execution flag
  xbeeDM.setChannel(network.channel);
  if (xbeeDM.error_AT)
  {
    error = F("ERROR in setChannel %d");
    goto exit;
  }

  // set PANID, check AT commmand execution flag
  xbeeDM.setPAN(network.panid);
  if (xbeeDM.error_AT)
  {
    error = F("ERROR in setPAN %d");
    goto exit;
  }

  // set encryption mode (1:enable; 0:disable), check AT commmand execution flag
  // XXX Should we use encryption
  xbeeDM.setEncryptionMode(0);
  if (xbeeDM.error_AT)
  {
    error = F("ERROR in setPAN %d");
    goto exit;
  }

  // set encryption key, check AT commmand execution flag
  xbeeDM.setLinkKey(encryptionKey);
  if (xbeeDM.error_AT)
  {
    error = F("ERROR in setLinkKey %d");
    goto exit;
  }

  // write values to XBee module memory, check AT commmand execution flag
  xbeeDM.writeValues();
  if (xbeeDM.error_AT)
  {
    error = F("ERROR in writeValues %d");
    goto exit;
  }

exit:
  xbeeDM.OFF();
  if (error)
  {
    cr.print(error, xbeeDM.error_AT);
  }
}
