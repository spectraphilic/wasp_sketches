#include <WaspUIO.h>

// David, few comments:
// The frame do not need to be send to the network, or if so, we need to give a name to the mote,
// and update its time with GPS. The data can be extracted via the SD to make it easier. 
// 30s delay between measurements can be good. Also, I can keep the mote on all the time, and that when I plug the battery, 
// it automatically goes into recording rssi, timestamp, and lat/long/altitude. To do so, the name, 
// network and so forth can be set within the script before uploading


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
  char time[12];
  uint32_t wait = 60 - UIO.getEpochTime() % 60;

  // Version with delay
  info(F("delay(%ds)"), wait); // Wait until the next minute
  delay(wait * 1000);
  if (ping() == 0)
  {
    // Frame
    UIO.createFrame();
    ADD_SENSOR(SENSOR_RSSI, rssi);
    if (UIO.gps(false, true) == 0)
    {
      UIO.frame2Sd();
      // Send frame (TODO)
    }
  }
}
