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
  xbeeDM.setSendingRetries(1);

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


int WaspUIO::xbee_ping(int &rssi)
{
  bool success = false;

  USB.OFF();

  // Action
  if (xbeeDM.ON() == 0)
  {
    if (xbeeDM.send((char*)UIO.xbee.rx_address, (char*)"ping") == 0)
    {
      if (xbeeDM.getRSSI() == 0)
      {
        rssi = xbeeDM.valueRSSI[0];
        rssi *= -1;
        success = true;
      }
    }
  }
  xbeeDM.OFF();

  // Print
  USB.ON();
  USB.flush();
  if (! success)
  {
    error(F("ping() Error"));
    return 1;
  }

  info(F("RSSI(dBm) = %d"), rssi);
  return 0;
}

/**
 * Function to communicate through OTA with remote unit
 *
 * Parameters: int OTA_duration  - duration in minute of the time window for OTA access to the unit
 */
void WaspUIO::OTA_communication(int OTA_duration)
{
  unsigned long start;
  unsigned long duration_ms;

  // OTA_duration is given in minutes
  duration_ms = OTA_duration * 60 * 1000;

  start = millis();
  do
  {
    if( xbeeDM.available() )
    {
      xbeeDM.treatData();
      // Keep inside this loop while a new program is being received
      while( xbeeDM.programming_ON  && !xbeeDM.checkOtapTimeout() )
      {
        if( xbeeDM.available() )
        {
          xbeeDM.treatData();
        }
      }
    }
  } while (! cr.timeout(start, duration_ms));
}
#endif
