#include "WaspUIO.h"


#if WITH_LORA
void WaspUIO::loraInit()
{
  int err;

  sx1272.ON();

  // 1. Configure the module
  // Note: configuration is not save in the module, so it must be configured every
  // time the module is switched ON
  err = sx1272.setMode(LORA_MODE);
  if (err)
  {
    cr.println(F("sx1272.setMode(..) error=%d"), err);
    goto exit;
  }

  err = sx1272.setChannel(LORA_CHANNEL);
  if (err)
  {
    cr.println(F("sx1272.setChannel(..) error=%d"), err);
    goto exit;
  }

  err = sx1272.setNodeAddress(lora_address); // 1 (Gateway) or 2-255
  if (err)
  {
    cr.println(F("sx1272.setNodeAddress(%u) error=%d"), lora_address, err);
    goto exit;
  }

  err = sx1272.setPower(LORA_POWER);
  if (err)
  {
    cr.println(F("sx1272.setPower(%u) error=%d"), lora_address, err);
    goto exit;
  }

  // 2. Read some parameters
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
  // TODO This closes SPI, verify it doesn't produce errors with other SPI
  // devices
  sx1272.OFF();
}
#endif
