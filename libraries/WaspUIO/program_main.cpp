#include "WaspUIO.h"


/**
 * Main task
 */

CR_TASK(taskMain)
{
  static tid_t health_id, sensors_id, network_id, gps_id;

  CR_BEGIN;

  // Create the first frame
  UIO.createFrame(true);

  // Sensors
  CR_SPAWN2(taskHealthFrame, health_id);
  CR_SPAWN2(taskSensors, sensors_id);

  // Network
  if ((UIO.batteryLevel > 30) && UIO.action(1, RUN_NETWORK))
  {
    CR_SPAWN2(taskNetwork, network_id);
    CR_JOIN(network_id);
  }

  CR_JOIN(health_id);
  CR_JOIN(sensors_id);

  // GPS (Once every 3 days)
  // The RTC is DS3231SN (v12) or DS1337C (v15), its accuracy is not enough
  // for our networking requirements, so we have to sync it once a day. See
  // http://hycamp.org/private-area/waspmote-rtc/
  if (UIO.hasGPS && UIO.minute == 0 && (UIO.day % 3 == 0))
  {
    CR_SPAWN2(taskGps, gps_id);
    CR_JOIN(gps_id);
  }

  // Save the last frame, if there is something to save
  if (frame.numFields > 1)
  {
    UIO.frame2Sd();
  }

  //CR_SPAWN(taskSlow);

  CR_END;
}


/**
 * This task is only to test the watchdog reset.
 */

CR_TASK(taskSlow)
{
  CR_BEGIN;

  // Wait a little bit so this is executed last
  CR_DELAY(12000);

  warn(F("Start slow task"));
  delay(5 * 60000); // 5 minutes
  warn(F("End slow task"));

  CR_END;
}
