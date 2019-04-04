#include "WaspUIO.h"
#include <assert.h>


/**
 * Retturn true if the given sensor is to be read now.
 */
bool WaspUIO::action(uint8_t n, ...)
{
  va_list args;
  bool yes = false;

  uint32_t minutes = epochTime / 60; // minutes since the epoch

  va_start(args, n);
  for (; n; n--)
  {
    int idx = va_arg(args, int);
    assert(idx < RUN_LEN); // TODO Define __assert

    uint16_t value = actions[idx] * cooldown;
    if (value > 0)
    {
      if (minutes % value == 0)
      {
        yes = true;
        break;
      }
    }
  }
  va_end(args);

  return yes;
}

/**
 * Main task
 */

CR_TASK(taskMain)
{
  static tid_t health_id, sensors_id, network_id, gps_id;

  CR_BEGIN;

  // Create the first frame
  UIO.createFrame(true);

  // XBee network. We do this one first to run it as predictable as possible,
  // to improve the chances for neighbor motes to be awake at the same time.
  // Do not use network and sensors at the same time, we have observed issues
  // in the past with SDI-12.
#if WITH_XBEE
  if ((UIO.battery > BATTERY_LOW) && UIO.action(1, RUN_NETWORK))
  {
    if (UIO.networkType == NETWORK_XBEE)
    {
      CR_SPAWN2(taskNetworkXBee, network_id);
      CR_JOIN(network_id);
    }
  }
#endif

  // Sensors
  CR_SPAWN2(taskHealthFrame, health_id);
  CR_SPAWN2(taskSensors, sensors_id);
  CR_JOIN(health_id);
  CR_JOIN(sensors_id);

  // GPS
  // The RTC is DS1337C (v15), its accuracy is not enough for our networking
  // requirements, so we have to sync it once in a while.
  // http://hycamp.org/private-area/waspmote-rtc/
  if (UIO.action(1, RUN_GPS))
  {
#if WITH_4G
    if (UIO.hasGPS & GPS_4G)
    {
      CR_SPAWN2(taskGPS4G, gps_id);
      CR_JOIN(gps_id);
    }
#endif
#if WITH_GPS
    if (UIO.hasGPS == GPS_YES)
    {
      CR_SPAWN2(taskGPS, gps_id);
      CR_JOIN(gps_id);
    }
#endif
  }

  // Save the last frame, if there is something to save
  if (frame.numFields > 1) { UIO.frame2Sd(); }


  // Networks that don't require mote syncrhonization run last.
  // They are often slower, specially satellite netwokrs (iridium), so they
  // may disturb sensor sampling if run earlier.
#if WITH_4G
  if ((UIO.battery > BATTERY_LOW) && UIO.action(1, RUN_NETWORK))
  {
    if (UIO.networkType == NETWORK_4G)
    {
      CR_SPAWN2(taskNetwork4G, network_id);
      CR_JOIN(network_id);
    }
  }
#endif
#if WITH_IRIDIUM
  if ((UIO.battery > BATTERY_LOW) && UIO.action(1, RUN_NETWORK))
  {
    if (UIO.networkType == NETWORK_IRIDIUM)
    {
      CR_SPAWN2(taskNetworkIridium, network_id);
      CR_JOIN(network_id);
    }
  }
#endif

  // Uncomment this to verify the watchdog works
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
