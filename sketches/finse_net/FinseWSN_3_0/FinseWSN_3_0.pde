

/*
 SCRIPT for Finse network, to synchronize DM network, and read basic set of sensors
 April 2017, Simon Filhol
*/

#include <WaspUIO.h>


void setup()
{
  // Boot process
  UIO.boot();

  // Boot frame
  frame.setID(UIO.name);
  frame.createFrameBin(BINARY); // TODO Move this logic to UIO.createFrame
  frame.setFrameType(INFORMATION_FRAME_V15 + EVENT_FRAME);
  UIO.addSensor(SENSOR_TST, UIO._epoch);
  UIO.saveFrame();
  #if WITH_XBEE
  frame.setID((char*)""); // We only want to send the name once
  #endif

  // Log configuration
  char buffer[150];
  size_t size = sizeof(buffer);
  log_info("Time      : %s", UIO.pprintTime(buffer, size));
  log_info("Id        : %s Version=%c Name=%s", UIO.pprintSerial(buffer, sizeof buffer), _boot_version, UIO.name);
  log_info("Battery   : %s", UIO.pprintBattery(buffer, size));
  log_info("Hardware  : board=%s SD=%d GPS=%d", UIO.pprintBoard(buffer, size), UIO.hasSD, UIO.hasGPS);
  #if WITH_XBEE
  if (UIO.lan_type == LAN_XBEE)
  { log_info("XBee      : %s", UIO.pprintXBee(buffer, size)); }
  #endif
  #if WITH_LORA
  if (UIO.lan_type == LAN_LORA)
  { log_info("Lora      : %s", UIO.pprintLora(buffer, size)); }
  #endif
  #if WITH_4G
  if (UIO.wan_type == WAN_4G)
  { log_info("4G        : %s", UIO.pprint4G(buffer, size)); }
  #endif
  #if WITH_IRIDIUM
  if (UIO.wan_type == WAN_IRIDIUM)
  { log_info("Iridium   : %s", UIO.pprintIridium(buffer, size)); }
  #endif
  log_info("Frames    : %s", UIO.pprintFrames(buffer, size));
  log_info("Log       : level=%s output=%s", cr.loglevel2str(cr.loglevel), UIO.pprintLog(buffer, size));
  log_info("Actions   : %s", UIO.pprintActions(buffer, size));
}

void loop()
{
  UIO.deepSleep();

  // We expect *only* Alarm 1
  if (intFlag & ~RTC_INT)
  {
    log_error("Expected *only* RTC_INT got %d", intFlag);
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

  log_info("Loop done in %lu ms", cr.millisDiff(UIO._loop_start));
}
