#include "WaspUIO.h"


/**
 * Main task
 */

CR_TASK(taskMain)
{
  static tid_t health_id, sensors_id, network_id, gps_id;

  CR_BEGIN;

  // Create the first frame
  UIO.createFrame();

  // Network
  // First the network, then the sensors. This is to avoid interferences as I
  // have found between receiving frames and the SDI-12 bus.  We do first the
  // network because we need it to be run in a predictable time.
  if ((UIO.battery > BATTERY_LOW) && UIO.action(1, RUN_NETWORK))
  {
    if (UIO.networkType == NETWORK_XBEE)
    {
      CR_SPAWN2(taskNetworkXBee, network_id);
    }
    else if (UIO.networkType == NETWORK_4G)
    {
      CR_SPAWN2(taskNetwork4G, network_id);
    }
    CR_JOIN(network_id);
  }

  // Sensors
  CR_SPAWN2(taskHealthFrame, health_id);
  CR_SPAWN2(taskSensors, sensors_id);
  CR_JOIN(health_id);
  CR_JOIN(sensors_id);

  // GPS
  // The RTC is DS1337C (v15), its accuracy is not enough for our networking
  // requirements, so we have to sync it once in a while.
  // http://hycamp.org/private-area/waspmote-rtc/
  if (UIO.hasGPS && UIO.action(1, RUN_GPS))
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
