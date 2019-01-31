#include "WaspUIO.h"

void WaspUIO::networkInit()
{
#if WITH_XBEE
  if (networkType == NETWORK_XBEE) { xbeeInit(); }
#endif

#if WITH_4G
  if (networkType == NETWORK_4G) { _4GInit(); }
#endif

#if WITH_IRIDIUM
  if (networkType == NETWORK_IRIDIUM) { iridiumInit(); }
#endif
}


/*
 * XBee
 */

#if WITH_XBEE
void WaspUIO::xbeeInit()
{
  const __FlashStringHelper * error = NULL;

  // init XBee
  if (xbeeDM.ON())
  {
    error = F("ERROR xbeeDM.ON()");
    return;
  }

  delay(50);

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
    error = F("ERROR in setChannel %d");
    goto exit;
  }

  // set PANID, check AT commmand execution flag
  xbeeDM.setPAN(xbee.panid);
  if (xbeeDM.error_AT)
  {
    error = F("ERROR in setPAN %d");
    goto exit;
  }

  // Disable link layer encryption
  xbeeDM.setEncryptionMode(0);
  if (xbeeDM.error_AT)
  {
    error = F("ERROR in setPAN %d");
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


/******************************************************************************/
/* Function to communicate through OTA with remote unit
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


/*
 * 4G
 */

#if WITH_4G
void WaspUIO::_4GInit()
{
}

uint8_t WaspUIO::_4GStart()
{
  uint8_t err, status;
  char pin_str[5];

  // Check pin number
  if (pin == 0 || pin > 9999)
  {
    error(F("Bad pin number (%u), set pin in the menu"), pin);
    return 1;
  }
  snprintf(pin_str, sizeof pin_str, "%04d", pin);

  // Switch on (11s)
  debug(F("4G Switch on"));
  err = _4G.ON();
  if (err)
  {
    error(F("_4G.ON error=%d %d"), err, _4G._errorCode);
    return 1;
  }

  // Enter PIN (0.2s)
  debug(F("4G Enter PIN"));
  status = _4G.checkPIN();
  if (status == 0)
  {
    debug(F("PIN READY"));
  }
  else if (status == 1)
  {
    err = _4G.enterPIN(pin_str);
    if (err)
    {
      pin = 0; updateEEPROM(EEPROM_UIO_PIN, pin); // Reset pin to avoid trying again
      error(F("_4G.enterPIN(%s) error=%d %d"), pin_str, err, _4G._errorCode);
    }
  }
  else
  {
    error(F("unexpected SIM status=%%hhu"), status);
    err = 1;
  }

  // Check data connection: usually ~11s sometimes close to 120s (a 2nd call
  // would take 0.13s)
  if (err == 0)
  {
    debug(F("4G Check data connection"));
    err = _4G.checkDataConnection(120);
    if (err)
    {
      error(F("_4G.checkDataConnection error=%d %d"), err, _4G._errorCode);
    }
    else
    {
      debug(F("4G Data connection OK"));
    }
  }

  // Switch off if error
  if (err)
  {
    _4G.OFF();
  }

  return err;
}

uint8_t WaspUIO::_4GStop()
{
  _4G.OFF();
  return 0;
}
#endif


/*
 * Iridium
 */

#if WITH_IRIDIUM
/*
 * Read the firmware version
 */
void WaspUIO::iridiumInit()
{
  int status;
  uint8_t size = sizeof iridium_fw;

  status = iridium_start();
  if (status != ISBD_SUCCESS)
  {
    snprintf_F(iridium_fw, size, F("err %d"), status);
  }
  else
  {
    status = iridium.getFirmwareVersion(iridium_fw, size);
    if (status != ISBD_SUCCESS)
    {
      snprintf_F(iridium_fw, size, F("err %d"), status);
    }
  }

  iridium_stop();
}
#endif
