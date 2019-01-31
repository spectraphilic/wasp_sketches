/*
 * Short Bus Data protocol (SBD)
 * Packet size (TX): 340bytes
 * Pins Iridium to Waspomote:
 * 5V (BATTERY) / Gnd / TX (2TX) / RX (2RX) / Sleep (DIGITAL) / RING (not use)
 * RX/TX to UART1 (19200 bauds)
 */

#include "WaspUIO.h"


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
    error(F("ERROR iridium.begin() error=%d"), status);
    iridium_stop();
  }

  return status;
}

int WaspUIO::iridium_stop()
{
  int status = iridium.sleep();
  if (status != ISBD_SUCCESS)
  {
    error(F("ERROR iridium.sleep() error=%d"), status);
  }

  closeSerial(UART1);
  Utils.muxOFF1();
  //Utils.setMux(LOW, LOW);

  return status;
}
