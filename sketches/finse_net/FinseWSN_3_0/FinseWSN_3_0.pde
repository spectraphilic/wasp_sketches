
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
  char name[17];
  char buffer[150];
  size_t size = sizeof(buffer);

  // Boot process
  USB.ON();
  UIO.boot();
  UIO.clint();   // Command line interface
  USB.OFF();

  // Boot frame
  Utils.getID(name);
  frame.setID(name);
  frame.createFrameBin(BINARY); // TODO Move this logic to UIO.createFrame
  frame.setFrameType(INFORMATION_FRAME_V15 + EVENT_FRAME);
  UIO.addSensor(SENSOR_TST, UIO.epochTime);
  UIO.frame2Sd();
#if WITH_XBEE
  frame.setID((char*)""); // We only want to send the name once
#endif

  // Log configuration
  info(F("Id        : %s Version=%c Name=%s"), UIO.pprintSerial(buffer, sizeof buffer), _boot_version, name);
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

  info(F("Frames    : %s"), UIO.pprintFrames(buffer, size));
  info(F("Log       : level=%s output=%s"), cr.loglevel2str(cr.loglevel), UIO.pprintLog(buffer, size));
  info(F("Actions   : %s"), UIO.pprintActions(buffer, size));
}

void loop()
{
  UIO.deepSleep();
  UIO.onLoop();

  char buffer[50];

  info(F("*** Loop %u battery=%s"), UIO.nloops, UIO.pprintBattery(buffer, sizeof buffer));

  // Low battery level: do nothing
  if (UIO.battery == BATTERY_LOW)
  {
    debug(F("*** Low battery: skip"));
    return;
  }

  // Check RTC interruption
  if (intFlag & RTC_INT)
  {
    intFlag &= ~(RTC_INT); RTC.alarmTriggered = 0;

    cr.reset();
    cr.spawn(taskMain);
    cr.run();
  }

  if (intFlag)
  {
    warn(F("Unexpected interruption %d"), intFlag);
    intFlag = 0;
  }

  uint32_t cpu_time = cr.millisDiff(UIO.start);
  info(F("Loop done in %lu ms"), cpu_time);
//char alarmTime[12];
//UIO.getNextAlarm(alarmTime);
//info(F("NEXT ALARM %s"), alarmTime);
}
