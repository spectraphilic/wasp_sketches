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

  // Logging available from here
  UIO.startSD();

  // Log configuration
  char buffer[150];
  size_t size = sizeof(buffer);
  info(F("Booting... (battery level is %d)"), UIO.batteryLevel);
  info(F("Config Logging: level=%s output=%s"), cr.loglevel2str(cr.loglevel), UIO.menuFormatLog(buffer, size));
  info(F("Config Network: %s"), UIO.menuFormatNetwork(buffer, size));
  info(F("Config Wakeup : %d minutes"), UIO.wakeup_period);
  info(F("Config Sensors: %s"), UIO.menuFormatSensors(buffer, size));

  // Set time from GPS if wrong time is detected
  // XXX Do this unconditionally to update location?
/*
  if (UIO.epochTime < 1483225200) // 2017-01-01 arbitrary date in the past
  {
    warn(F("Wrong time detected, updating from GPS"));
    actionGps();
  }
*/

  // Calculate first alarm (requires batteryLevel)
  char alarmTime[12]; // "00:00:00:00"
  UIO.getNextAlarm(alarmTime);

  // Go to sleep
  info(F("Boot done, go to sleep"));
  UIO.stopSD();
  PWR.deepSleep(alarmTime, RTC_ABSOLUTE, RTC_ALM1_MODE4, ALL_OFF);
}

void loop()
{
  UIO.onLoop();
  UIO.startSD(); // Logging starts here

  // Check RTC interruption
  if (intFlag & RTC_INT)
  {
    // Battery level, do nothing if too low
    if (UIO.batteryLevel <= 30) {
      debug(F("RTC interruption, low battery = %d"), UIO.batteryLevel);
      goto sleep;
    }

    info(F("*** RTC interruption, battery level = %d"), UIO.batteryLevel);
    //Utils.blinkGreenLED(); // blink green once every minute to show it is alive

    // Create the first frame
    UIO.createFrame(true);

    cr.reset();
    cr.spawn(taskHealthFrame);
    cr.spawn(taskSensors);
    if (UIO.featureNetwork && UIO.action(1, 12)) // 12 x 5min = 1hour
    {
      cr.spawn(taskNetwork);
    }
    // GPS (Once a day)
    // The RTC is DS3231SN which at -40 C has an accuracy of 3.5ppm, that's
    // about 0.3s per day, with aging it may be worse.
    // See https://www.maximintegrated.com/en/app-notes/index.mvp/id/3566
    //
    // As clock sync between the devices is critical for the network to work
    // properly, we update the RTC time from GPS daily. But the GPS draws power,
    // so this may need to be tuned.
//  if (UIO.time.minute == 0 && UIO.time.hour == 0);
//  {
//    cr.spawn(taskGps);
//  }
//  cr.spawn(taskSlow);
    cr.run();

    // Save the last frame, if there is something to save
    if (frame.numFields > 1)
    {
      UIO.frame2Sd();
    }
  }
  else
  {
    warn(F("Unexpected interruption %d"), intFlag);
  }

sleep:
  // Calculate first alarm (requires batteryLevel)
  char alarmTime[12]; // "00:00:00:00"
  UIO.getNextAlarm(alarmTime);

  uint32_t cpu_time = cr.millisDiff(UIO.start);
  info(F("Loop done in %lu ms (CPU time %lu ms)."), cpu_time + cr.sleep, cpu_time);
  UIO.stopSD(); // Logging ends here

  // Clear interruption flag & pin
  clearIntFlag();
  PWR.clearInterruptionPin();

  // Set whole agri board and waspmote to sleep, until next alarm.
  PWR.deepSleep(alarmTime, RTC_ABSOLUTE, RTC_ALM1_MODE4, ALL_OFF);
}
