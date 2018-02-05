#include "WaspUIO.h"


void WaspUIO::initNet()
{
  uint8_t addressing = UNICAST_64B;
  uint8_t value = network.panid[1]; // panid low byte
  const __FlashStringHelper * error = NULL;

  // Addressing
  memcpy_P(&network, &networks[value], sizeof network);
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
    cr.println(error, xbeeDM.error_AT);
  }
}
