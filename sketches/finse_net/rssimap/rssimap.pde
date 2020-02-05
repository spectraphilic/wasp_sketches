#include <WaspUIO.h>

/*
 * Every 30s this sketch produces a frame with the RSSI, GPS location and
 * altitude.
 *
 * A frame is only generated if the mote can get both the RSSI and GPS. Note
 * that getting the GPS data may take more than 30s. For these 2 reasons the
 * final values may not be once every 30s.
 *
 * The frame is not sent, just stored. When done running the main sketch can be
 * used to send the frames.
 *
 * To get the RSSI we have to ping (send a frame). The frame is sent to the hub
 * (Pi), but the RSSI only concerns the first hop. The hub will ignore this
 * frame, just log it.
 *
 * Please before using this sketch first configure the mote properly with the
 * main sketch, specially the time and name. And logging to the SD, so we can
 * debug it later.
 */


uint8_t getPowerLevel()
{
  bool success = false;
  uint8_t powerLevel;

  info(F("getPowerLevel() ..."));
  USB.OFF();

  // Action
#if WITH_XBEE
  if (xbeeDM.ON() == 0 && xbeeDM.getPowerLevel() == 0)
  {
    powerLevel = xbeeDM.powerLevel;
    success = true;
  }
  xbeeDM.OFF();
#elif WITH_LORA
  if (UIO.loraStart() == 0 && sx1272.getPower() == 0)
  {
    powerLevel = sx1272._power;
    success = true;
  }
  UIO.loraStop();
#endif

  // Print
  USB.ON();
  USB.flush();
  if (success)
  {
    info(F("power level = %hhu"), powerLevel);
    return 0;
  }
  else
  {
    error(F("Failed to read the power level"));
    return 1;
  }
}


void setup()
{
  USB.ON();
  UIO.boot();
  USB.println();

  // SD
  UIO.startSD();
  RTC.ON();

  // XBee Power level
  getPowerLevel();

  // GPS
  if (GPS.ON() == 0)
  {
    error(F("GPS.ON() Error: Shut down!!"));
    PWR.deepSleep("01:00:00:00", RTC_OFFSET, RTC_ALM1_MODE2, ALL_OFF);
  }
}


void loop()
{
  uint32_t time, wait;
  int err;

  // Wait until the next 30s slot
  wait = 30 - UIO.getEpochTime() % 30; // Every 30s
  info(F("delay(%ds)"), wait); // Wait until the next minute
  delay(wait * 1000);

  // Get data and create frame
  time = UIO.getEpochTime();
  info(F("ping() ..."));

#if WITH_XBEE
  err = UIO.xbeeSend(UIO.xbee.rx_address, "ping");
#elif WITH_LORA
  err = UIO.loraSend(UIO.lora_dst, "ping", true);
#endif

  if (err == 0)
  {
    // Frame
    frame.createFrameBin(BINARY);
    ADD_SENSOR(SENSOR_TST, time);
    ADD_SENSOR(SENSOR_RSSI, UIO.rssi);
#if WITH_LORA
    //ADD_SENSOR(SENSOR_SNR, UIO.snr);
#endif
    if (UIO.gps(false, true) == 0)
    {
      UIO.saveFrame();
    }
  }
}
