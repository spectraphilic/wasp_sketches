#include "WaspUIO.h"


#if WITH_XBEE
void WaspUIO::xbeeInit()
{
  PGM_P err = NULL;

  // XBee network
  memcpy_P(&xbee, &xbees[xbee_network], sizeof xbee);

  // init XBee
  if (xbeeDM.ON())
  {
    cr_printf("ERROR xbeeDM.ON()\n");
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
    err = PSTR("ERROR in setChannel %d\n");
    goto exit;
  }

  // set PANID, check AT commmand execution flag
  xbeeDM.setPAN(xbee.panid);
  if (xbeeDM.error_AT)
  {
    err = PSTR("ERROR in setPAN %d\n");
    goto exit;
  }

  // Disable link layer encryption
  xbeeDM.setEncryptionMode(0);
  if (xbeeDM.error_AT)
  {
    err = PSTR("ERROR in setPAN %d\n");
    goto exit;
  }

  // write values to XBee module memory, check AT commmand execution flag
  xbeeDM.writeValues();
  if (xbeeDM.error_AT)
  {
    err = PSTR("ERROR in writeValues %d\n");
    goto exit;
  }

exit:
  xbeeDM.OFF();
  if (err)
  {
    cr.printf_P(err, xbeeDM.error_AT);
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
    log_error("xbeeSend failed error=%d", err);
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
