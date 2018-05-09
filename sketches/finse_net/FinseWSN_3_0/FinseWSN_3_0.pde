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
  char hw[5];
  char sw[9];
  size_t size = sizeof(buffer);

  // Boot process
  USB.ON();
  cr.print(F("."));
  UIO.onSetup();
  cr.print(F("."));
  UIO.onLoop();
  cr.print(F("."));
  UIO.initNet(); // Network
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
  frame.addSensorBin(SENSOR_TST, UIO.epochTime);
  UIO.frame2Sd();
  frame.setID((char*)""); // We only want to send the name once

  // Set time from GPS if wrong time is detected
  // XXX Do this unconditionally to update location?
  if (UIO.epochTime < 1483225200) // 2017-01-01 arbitrary date in the past
  {
    warn(F("Wrong time detected, updating from GPS"));
    taskGps();
  }

  // Log configuration
  Utils.hex2str(xbeeDM.hardVersion, hw, 2);
  Utils.hex2str(xbeeDM.softVersion, sw, 4);

  info(F("Id        : %s Version=%c Name=%s"), UIO.pprintSerial(buffer, sizeof buffer), _boot_version, name);
  info(F("Battery   : %s"), UIO.pprintBattery(buffer, size));
  info(F("Board     : %s"), UIO.pprintBoard(buffer, size));
  info(F("XBee      : %s hw=%s sw=%s"), UIO.myMac, hw, sw);
  info(F("Autodetect: SD=%d GPS=%d"), UIO.hasSD, UIO.hasGPS);
  info(F("Logging   : level=%s output=%s"), cr.loglevel2str(cr.loglevel), UIO.pprintLog(buffer, size));
  info(F("Network   : %s"), UIO.pprintNetwork(buffer, size));
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
