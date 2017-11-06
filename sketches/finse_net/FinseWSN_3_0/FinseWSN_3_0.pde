/*
 SCRIPT for Finse network, to synchronize DM network, and read basic set of sensors
 April 2017, Simon Filhol
 Script description:
 */

// 1. Include Libraries
#include <WaspUIO.h>
#include <WaspFrame.h>

// 2. Definitions


// 3. Global variables declaration

void setup()
{
  UIO.onSetup();
  UIO.onLoop();

  // Interactive mode
  USB.ON();
  UIO.initNet();
  UIO.menu();
  USB.OFF();

  // Log configuration
  char buffer[150];
  size_t size = sizeof(buffer);
  info(F("Booting (%c)..."), _boot_version);
  info(F("Hardware: SD=%d GPS=%d"), UIO.hasSD, UIO.hasGPS);
  info(F("Config Battery: %s (%d %%)"), UIO.menuFormatBattery(buffer, size), UIO.batteryLevel);
  info(F("Config Logging: level=%s output=%s"), cr.loglevel2str(cr.loglevel), UIO.menuFormatLog(buffer, size));
  info(F("Config Network: %s"), UIO.menuFormatNetwork(buffer, size));
  info(F("Config Wakeup : %d minutes"), UIO.wakeup_period);
  info(F("Config Sensors: %s"), UIO.menuFormatSensors(buffer, size));

  // Set time from GPS if wrong time is detected
  // XXX Do this unconditionally to update location?
  if (UIO.epochTime < 1483225200) // 2017-01-01 arbitrary date in the past
  {
    warn(F("Wrong time detected, updating from GPS"));
    taskGps();
  }
  info(F("Boot done, go to sleep"));
  UIO.stopSD();
}

void loop()
{
  UIO.deepSleep();
  UIO.onLoop();

  // Low battery level: do nothing
  if (UIO.batteryType == 1)
  {
    if (UIO.batteryLevel <= 30)
    {
      debug(F("*** Loop skip (battery %d %%)"), UIO.batteryLevel);
      return;
    }
  }

  // Logging starts here
  info(F("*** Loop start (battery %d %%)"), UIO.batteryLevel);

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
