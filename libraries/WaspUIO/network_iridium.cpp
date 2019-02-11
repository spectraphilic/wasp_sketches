#include "WaspUIO.h"


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


int WaspUIO::iridium_ping()
{
  status = UIO.iridium_start(); // This takes ~700ms
  if (status != ISBD_SUCCESS)
  {
    return 1;
  }

  // Quality
  status = iridium.getSignalQuality(quality); // This takes ~4s
  if (status != ISBD_SUCCESS)
  {
    cr.println(F("iridium.getSignalQuality(..) error=%d"), status);
    UIO.iridium_stop();
    return 1;
  }
  cr.println(F("Quality = %d"), quality);

  // Send
  status = iridium.sendSBDBinary((uint8_t*)SD.buffer, size);
  if (status != ISBD_SUCCESS)
  {
    cr.println(F("iridium.sendSBDBinary(..) error=%d"), status);
    return 1;
  }

  // Quality
  status = iridium.getSignalQuality(quality); // This takes ~4s
  if (status != ISBD_SUCCESS)
  {
    cr.println(F("iridium.getSignalQuality(..) error=%d"), status);
    UIO.iridium_stop();
    return 1;
  }
  cr.println(F("Quality = %d"), quality);


  return 0;
}


#endif
