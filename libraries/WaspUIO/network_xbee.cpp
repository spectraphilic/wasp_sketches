#include "WaspUIO.h"


#if WITH_XBEE
void WaspUIO::xbeeInit()
{
  const __FlashStringHelper * err = NULL;

  // init XBee
  if (xbeeDM.ON())
  {
    cr.println(F("ERROR xbeeDM.ON()"));
    return;
  }

  xbeeDM.getHardVersion();
  xbeeDM.getSoftVersion();
  xbeeDM.getOwnMac();

  // Reduce the number of retries (default is 3)
  // There're as well 10 retries (by default) at the lower level.
  xbeeDM.setSendingRetries(2);

  // Set channel, check AT commmand execution flag
  xbeeDM.setChannel(xbee.channel);
  if (xbeeDM.error_AT)
  {
    err = F("ERROR in setChannel %d");
    goto exit;
  }

  // set PANID, check AT commmand execution flag
  xbeeDM.setPAN(xbee.panid);
  if (xbeeDM.error_AT)
  {
    err = F("ERROR in setPAN %d");
    goto exit;
  }

  // Disable link layer encryption
  xbeeDM.setEncryptionMode(0);
  if (xbeeDM.error_AT)
  {
    err = F("ERROR in setPAN %d");
    goto exit;
  }

  // write values to XBee module memory, check AT commmand execution flag
  xbeeDM.writeValues();
  if (xbeeDM.error_AT)
  {
    err = F("ERROR in writeValues %d");
    goto exit;
  }

exit:
  xbeeDM.OFF();
  if (err)
  {
    cr.println(err, xbeeDM.error_AT);
  }
}


// With XBee we always use network ACK
int WaspUIO::xbeeSend(const char* dst, const char* msg)
{
  int err;

  // UART0 is shared by USB and Socket0 (XBee, Lora)
  USB.OFF();

  // Switch ON
  bool isOn = xbeeDM.XBee_ON;
  if (! isOn)
  {
    err = xbeeDM.ON();
    if (err) goto exit;
  }

  err = xbeeDM.send((char*)dst, (char*)msg);
  if (err == 0)
  {
    err = xbeeQuality();
  }

exit:
  if (! isOn)
  {
    xbeeDM.OFF();
  }

  // Print
  //USB.ON();
  //USB.flush();
  if (err)
  {
    error(F("xbeeSend failed error=%d"), err);
    return 1;
  }

  return 0;
}

int WaspUIO::xbeeQuality()
{
  int err = xbeeDM.getRSSI();
  if (err == 0)
  {
    rssi = xbeeDM.valueRSSI[0] * -1;
  }
  return err;
}

#endif
