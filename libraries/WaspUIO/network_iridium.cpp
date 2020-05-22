/*
 * Short Bus Data protocol (SBD)
 * Packet size (TX): 340bytes
 * Pins Iridium to Waspomote:
 * 5V (BATTERY) / Gnd / TX (2TX) / RX (2RX) / Sleep (DIGITAL4) / RING (not use)
 * RX/TX to UART1 (19200 bauds)
 */

#include "WaspUIO.h"


#if WITH_IRIDIUM
bool ISBDCallback()
{
  return true; // Return false to cancel
}

// These ones are required, even if they do nothing, otherwise I get an
// infinite reboot loop
void ISBDConsoleCallback(IridiumSBD *device, char c)
{
  //USB.print(c);
}

void ISBDDiagsCallback(IridiumSBD *device, char c)
{
  //USB.print(c);
}


int WaspUIO::iridium_start()
{
  Utils.setMuxAux2();
  beginSerial(19200, UART1);
  iridium.setPowerProfile(IridiumSBD::DEFAULT_POWER_PROFILE);
  iridium.adjustSendReceiveTimeout(180);
  int status = iridium.begin(); // Wake up the 9602 and prepare it for communications.
  if (status != ISBD_SUCCESS)
  {
    log_error("ERROR iridium.begin() error=%d", status);
    iridium_stop();
  }

  return status;
}

int WaspUIO::iridium_stop()
{
  int status = iridium.sleep();
  if (status != ISBD_SUCCESS)
  {
    log_error("ERROR iridium.sleep() error=%d", status);
  }

  closeSerial(UART1);
  Utils.muxOFF1();
  //Utils.setMux(LOW, LOW);

  return status;
}


/*
 * Read the firmware version
 */
void WaspUIO::iridiumInit()
{
  uint8_t size = sizeof iridium_fw;

  int status = iridium_start();
  if (status != ISBD_SUCCESS)
  {
    cr_snprintf(iridium_fw, size, "err %d", status);
  }
  else
  {
    status = iridium.getFirmwareVersion(iridium_fw, size);
    if (status != ISBD_SUCCESS)
    {
      cr_snprintf(iridium_fw, size, "err %d", status);
    }
  }

  iridium_stop();
}


int WaspUIO::iridium_ping()
{
  int quality, size;

  log_info("Iridium ping...");
  int status = UIO.iridium_start(); // This takes ~700ms
  if (status != ISBD_SUCCESS)
  {
    return 1;
  }

  // Quality
  status = iridium.getSignalQuality(quality); // This takes ~4s
  if (status != ISBD_SUCCESS)
  {
    log_error("iridium.getSignalQuality(..) error=%d", status);
    UIO.iridium_stop();
    return 1;
  }
  log_info("Quality = %d", quality);

  // Send
  status = iridium.sendSBDText("ping");
  if (status != ISBD_SUCCESS)
  {
    log_error("iridium.sendSBDText(..) error=%d", status);
    return 1;
  }

  log_info("Success!");

  // Quality
  status = iridium.getSignalQuality(quality); // This takes ~4s
  if (status != ISBD_SUCCESS)
  {
    log_error("iridium.getSignalQuality(..) error=%d", status);
    UIO.iridium_stop();
    return 1;
  }
  log_debug("Quality = %d", quality);

  return 0;
}


#endif
