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

int WaspUIO::iridium_start()
{
  Utils.setMuxAux2();
  beginSerial(19200, UART1);
  iridium.setPowerProfile(IridiumSBD::DEFAULT_POWER_PROFILE);
  int status = iridium.begin(); // Wake up the 9602 and prepare it for communications.
  if (status != ISBD_SUCCESS)
  {
    cr.println(F("ERROR iridium.begin() error=%d"), status);
    iridium_stop();
  }
  else
  {
    //delay(500); // XXX Do we need this?
  }

  return status;
}

int WaspUIO::iridium_stop()
{
  int status = iridium.sleep();
  if (status != ISBD_SUCCESS)
  {
    cr.println(F("ERROR iridium.sleep() error=%d"), status);
  }

  closeSerial(UART1);
  Utils.muxOFF1();
  //Utils.setMux(LOW, LOW);

  return status;
}
