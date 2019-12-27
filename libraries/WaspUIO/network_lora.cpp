#include "WaspUIO.h"


#if WITH_LORA
int WaspUIO::loraStart()
{
  int err;

  err = sx1272.ON();
  if (err)
  {
    cr.println(F("sx1272.ON() error=%d"), err);
    return err;
  }

  // Configuration is not saved in the module, so the module must be configured
  // every time it's switched ON

  err = sx1272.setMode(lora_mode);
  if (err)
  {
    cr.println(F("sx1272.setMode(%u) error=%d"), lora_mode, err);
    goto exit;
  }

  err = sx1272.setChannel(LORA_CHANNEL);
  if (err)
  {
    cr.println(F("sx1272.setChannel(..) error=%d"), err);
    goto exit;
  }

  err = sx1272.setNodeAddress(lora_addr); // 1 (Gateway) or 2-255
  if (err)
  {
    cr.println(F("sx1272.setNodeAddress(%u) error=%d"), lora_addr, err);
    goto exit;
  }

  err = sx1272.setPower(LORA_POWER);
  if (err)
  {
    cr.println(F("sx1272.setPower(%u) error=%d"), lora_addr, err);
    goto exit;
  }

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

  // Read some parameters
  // TODO Verify these are not loaded automatically
  err = sx1272.getMaxCurrent();
  if (err)
  {
    cr.println(F("sx1272.getMaxCurrent() error=%d"), err);
    goto exit;
  }

  err = sx1272.getPayloadLength();
  if (err)
  {
    cr.println(F("sx1272.getPayloadLength() error=%d"), err);
    goto exit;
  }

  err = sx1272.getPreambleLength();
  if (err)
  {
    cr.println(F("sx1272.getPreambleLength() error=%d"), err);
    goto exit;
  }

exit:
  loraStop();
  return err;
}

#endif
