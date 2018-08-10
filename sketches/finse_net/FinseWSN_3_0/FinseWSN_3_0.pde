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
  cr.print(F("."));
  UIO.onSetup();
  cr.print(F("."));
  UIO.onLoop();
  cr.print(F("."));
  UIO.networkInit(); // Network
  UIO.clint();   // Command line interface
  USB.OFF();

  // Uptime frame
  Utils.getID(name);
  frame.setID(name);
  frame.createFrameBin(BINARY);
  if (_boot_version >= 'G')
  {
    frame.setFrameType(INFORMATION_FRAME_V15 + EVENT_FRAME);
  }
  else
  {
    frame.setFrameType(INFORMATION_FRAME_V12 + EVENT_FRAME);
  }
  UIO.addSensor(SENSOR_TST, UIO.epochTime);
  UIO.frame2Sd();
  //frame.setID((char*)""); // We only want to send the name once

  // Set time from GPS if wrong time is detected
  // XXX Do this unconditionally to update location?
  if (UIO.epochTime < 1483225200) // 2017-01-01 arbitrary date in the past
  {
    warn(F("Wrong time detected, updating from GPS"));
    taskGps();
  }

  // Log configuration
  info(F("Id        : %s Version=%c Name=%s"), UIO.pprintSerial(buffer, sizeof buffer), _boot_version, name);
  info(F("Battery   : %s"), UIO.pprintBattery(buffer, size));
  info(F("Hardware  : board=%s SD=%d GPS=%d"), UIO.pprintBoard(buffer, size), UIO.hasSD, UIO.hasGPS);

  if (UIO.networkType == NETWORK_XBEE)
  {
    info(F("XBee      : %s"), UIO.pprintXBee(buffer, size));
  }
  else if (UIO.networkType == NETWORK_4G)
  {
    info(F("4G        : %s"), UIO.pprint4G(buffer, size));
  }

  info(F("Log       : level=%s output=%s"), cr.loglevel2str(cr.loglevel), UIO.pprintLog(buffer, size));
  info(F("Actions   : %s"), UIO.pprintActions(buffer, size));

  info(F("Boot done, go to sleep"));
  UIO.stopSD();
}

void loop()
{
  UIO.deepSleep();
  RTC.ON(); // This fixes a bug with Maxbotix & SD card in some motes
  UIO.onLoop();

  char buffer[50];

  // Low battery level: do nothing
  if (UIO.battery == BATTERY_LOW)
  {
    debug(F("*** Loop skip (%s)"), UIO.pprintBattery(buffer, sizeof buffer));
    return;
  }

  // Logging starts here
  info(F("*** Loop start (%s)"), UIO.pprintBattery(buffer, sizeof buffer));

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
  info(F("Loop done in %lu ms (CPU time %lu ms)."), cpu_time + cr.sleep_time, cpu_time);
//char alarmTime[12];
//UIO.getNextAlarm(alarmTime);
//info(F("NEXT ALARM %s"), alarmTime);
  UIO.stopSD(); // Logging ends here
}
