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

// Global variables
uint8_t powerLevel;
int rssi;


uint8_t getPowerLevel()
{
  bool success = false;

  info(F("getPowerLevel() ..."));
  USB.OFF();

  // Action
  if (xbeeDM.ON() == 0)
  {
    if (xbeeDM.getPowerLevel() == 0)
    {
      powerLevel = xbeeDM.powerLevel;
      success = true;
    }
  }
  xbeeDM.OFF();

  // Print
  USB.ON();
  USB.flush();
  if (success)
  {
    info(F("powerLevel = %hhu"), powerLevel);
    return 0;
  }
  else
  {
    error(F("getPowerLevel() Error"));
    return 1;
  }
}

uint8_t ping()
{
  bool success = false;

  info(F("ping() ..."));
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
  if (success)
  {
    info(F("RSSI(dBm) = %d"), rssi);
    return 0;
  }
  else
  {
    error(F("ping() Error"));
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

  // Wait until the next 30s slot
  wait = 30 - UIO.getEpochTime() % 30; // Every 30s
  info(F("delay(%ds)"), wait); // Wait until the next minute
  delay(wait * 1000);

  // Get data and create frame
  time = UIO.getEpochTime();
  if (ping() == 0)
  {
    // Frame
    frame.createFrameBin(BINARY);
    ADD_SENSOR(SENSOR_TST, time);
    ADD_SENSOR(SENSOR_RSSI, rssi);
    if (UIO.gps(false, true) == 0)
    {
      UIO.frame2Sd();
    }
  }
}
