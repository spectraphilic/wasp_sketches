#include "WaspUIO.h"


#if WITH_LORA
int WaspUIO::loraStart()
{
  int err;

  err = sx1272.ON();
  if (err) { cr.println(F("sx1272.ON() error=%d"), err); return err; }

  // Configuration is not saved in the module, so the module must be configured
  // every time it's switched ON

  err = sx1272.setMode(lora_mode);
  if (err) { cr.println(F("sx1272.setMode(%u) error=%d"), lora_mode, err); goto exit; }

  err = sx1272.setChannel(LORA_CHANNEL);
  if (err) { cr.println(F("sx1272.setChannel(..) error=%d"), err); goto exit; }

  err = sx1272.setNodeAddress(lora_addr); // 1 (Gateway) or 2-255
  if (err) { cr.println(F("sx1272.setNodeAddress(%u) error=%d"), lora_addr, err); goto exit; }

  err = sx1272.setPower(LORA_POWER);
  if (err) { cr.println(F("sx1272.setPower(%u) error=%d"), lora_addr, err); goto exit; }

  // Used for sending
  err = sx1272.setHeaderON();
  if (err) { cr.println(F("sx1272.setHeaderON() error=%d"), err); goto exit; }

  err = sx1272.setCRC_ON();
  if (err) { cr.println(F("sx1272.setCRC_ON() error=%d"), err); goto exit; }

  // Read registers (read values should match the values above, for those that
  // have been set)
  err = sx1272.getRegs();
  if (err) { cr.println(F("sx1272.getRegs() error=%d"), err); goto exit; }

exit:
  if (err) { loraStop(); }
  return err;
}


void WaspUIO::loraStop()
{
  // This is safe. The SD card uses SPI as well, but SPI.close() will only
  // close SPI if all devices (SD and Socket0) are closed.
  sx1272.OFF();
}


int WaspUIO::loraInit()
{
  int err = loraStart();
  if (err) { return err; }

  loraStop();
  return err;
}

int WaspUIO::loraSend(uint8_t dst, const char* msg, bool ack)
{
  int err;

  debug(F("loraSend(dst=%d, msg=\"%s\", ack=%d)"), dst, msg, ack);

  // UART0 is shared by USB and Socket0 (XBee, Lora)
  USB.OFF();

  // Switch ON
  bool isOn = SPI.isSocket0;
  if (! isOn)
  {
    err = UIO.loraStart();
    if (err) goto exit;
  }

  if (ack) {
    err = sx1272.sendPacketTimeoutACKRetries(dst, (char*)msg);
    if (err == 0)
    {
      err = loraQuality();
    }
  } else {
    err = sx1272.sendPacketTimeout(dst, (char*)msg);
  }

exit:
  if (! isOn)
  {
    UIO.loraStop();
  }

  // Print
  //USB.ON();
  //USB.flush();
  if (err)
  {
    error(F("loraSend failed error=%d"), err);
    return 1;
  }

  return 0;
}

int WaspUIO::loraQuality()
{
  int err = sx1272.getRSSIpacket() or sx1272.getSNR();
  if (err == 0)
  {
    rssi = sx1272._RSSIpacket; // XXX Check this is a negative value
    snr = sx1272._SNR;
  }

  return err;
}

#endif
