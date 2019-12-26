
  /*
 SCRIPT for Finse network, to synchronize DM network, and read basic set of sensors
 April 2017, Simon Filhol
 Script description:
 */

// 1. Include Libraries
#include <WaspUIO.h>

// 2. Definitions


// 3. Global variables declaration

void setup()
{
  // Boot process
  UIO.boot();

  // Boot frame
  frame.setID(UIO.name);
  frame.createFrameBin(BINARY); // TODO Move this logic to UIO.createFrame
  frame.setFrameType(INFORMATION_FRAME_V15 + EVENT_FRAME);
  UIO.addSensor(SENSOR_TST, UIO._epoch);
  UIO.frame2Sd();
  #if WITH_XBEE
  frame.setID((char*)""); // We only want to send the name once
  #endif

  // Log configuration
  char buffer[150];
  size_t size = sizeof(buffer);
  info(F("Time      : %s"), UIO.pprintTime(buffer, size));
  info(F("Id        : %s Version=%c Name=%s"), UIO.pprintSerial(buffer, sizeof buffer), _boot_version, UIO.name);
  info(F("Battery   : %s"), UIO.pprintBattery(buffer, size));
  info(F("Hardware  : board=%s SD=%d GPS=%d"), UIO.pprintBoard(buffer, size), UIO.hasSD, UIO.hasGPS);
  #if WITH_XBEE
  if (UIO.networkType == NETWORK_XBEE)
  { info(F("XBee      : %s"), UIO.pprintXBee(buffer, size)); }
  #endif
  #if WITH_4G
  if (UIO.networkType == NETWORK_4G)
  { info(F("4G        : %s"), UIO.pprint4G(buffer, size)); }
  #endif
  #if WITH_IRIDIUM
  if (UIO.networkType == NETWORK_IRIDIUM)
  { info(F("Iridium   : %s"), UIO.pprintIridium(buffer, size)); }
  #endif
  #if WITH_LORA
  if (UIO.networkType == NETWORK_LORA)
  { info(F("Lora      : %s"), UIO.pprintLora(buffer, size)); }
  #endif
  info(F("Frames    : %s"), UIO.pprintFrames(buffer, size));
  info(F("Log       : level=%s output=%s"), cr.loglevel2str(cr.loglevel), UIO.pprintLog(buffer, size));
  info(F("Actions   : %s"), UIO.pprintActions(buffer, size));
}

void loop()
{
  UIO.deepSleep();

  // We expect *only* Alarm 1
  if (intFlag & ~RTC_INT)
  {
    error(F("Expected *only* RTC_INT got %d"), intFlag);
    intFlag = 0;
    return;
  }

  // Clean interruption flag
  intFlag &= ~(RTC_INT);
  PWR.clearInterruptionPin();

  // Low battery level: do nothing
  cr.reset();
  cr.spawn(taskMain);
  cr.run();

  info(F("Loop done in %lu ms"), cr.millisDiff(UIO._loop_start));
}
