#include "WaspUIO.h"
#include <assert.h>


/**
 * Retturn true if the given sensor is to be read now.
 */
bool WaspUIO::action(uint8_t n, ...)
{
  va_list args;
  bool yes = false;

  va_start(args, n);
  for (; n; n--)
  {
    int idx = va_arg(args, int);
    assert(idx < RUN_LEN); // TODO Define __assert
    Action action = actions[idx];

    if (action.type == action_minutes)
    {
      if (_epoch_minutes % (action.minute * cooldown) == 0)
      {
        yes = true; break;
      }
    }
    else if (action.type == action_hours)
    {
      uint32_t hours = _epoch_minutes / 60;
      uint32_t minutes = _epoch_minutes % 60;
      if (hours % (action.hour * cooldown) == 0 && minutes == action.minute)
      {
        yes = true; break;
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
    static tid_t health_id, sensors_id;
#if WITH_LORA || WITH_XBEE || WITH_4G || WITH_IRIDIUM
    static tid_t network_id;
#endif
#if WITH_GPS || WITH_4G
    static tid_t gps_id;
#endif

    CR_BEGIN;

    // Create the first frame
    UIO.createFrame(true);

    // Sensors
    CR_SPAWN2(taskHealthFrame, health_id);
    CR_SPAWN2(taskSensors, sensors_id);
    CR_JOIN(health_id);
    CR_JOIN(sensors_id);

    // GPS
    // The RTC is DS1337C (v15), its accuracy is not enough for our networking
    // requirements, so we have to sync it once in a while.
    // http://hycamp.org/private-area/waspmote-rtc/
    if ((UIO.battery > BATTERY_LOW) && UIO.action(1, RUN_GPS)) {
#if WITH_4G
        if (UIO.hasGPS & GPS_4G) {
            CR_SPAWN2(taskGPS4G, gps_id);
            CR_JOIN(gps_id);
        }
#endif
#if WITH_GPS
        if (UIO.hasGPS == GPS_YES) {
            CR_SPAWN2(taskGPS, gps_id);
            CR_JOIN(gps_id);
        }
#endif
    }

    // Save the last frame, if there is something to save
    if (frame.numFields > 1)
        UIO.saveFrame();

    // Don't use network and sensors at the same time. We have observed issues
    // in the past with SDI-12.

    // Ideally the XBee network should run in a differnt loop than the sensor
    // reading. So it runs at a predictable time, improving the chances of
    // success communication with neighbors. For instance read sensors every
    // 5min and run the XBee network every hour at :07 (in this example sensor
    // reading must finish in less than 2 minutes, otherwise the network loop
    // will be skept).
    if ((UIO.battery > BATTERY_LOW) && UIO.action(1, RUN_LAN)) {
#if WITH_XBEE
        if (UIO.lan_type == LAN_XBEE) {
            CR_SPAWN2(taskNetworkXBee, network_id);
            CR_JOIN(network_id);
        }
#endif

#if WITH_LORA
        if (UIO.lan_type == LAN_LORA) {
            CR_SPAWN2(taskNetworkLora, network_id);
            CR_JOIN(network_id);
        }
#endif
    }

    if ((UIO.battery > BATTERY_LOW) && UIO.action(1, RUN_WAN)) {
#if WITH_4G
        if (UIO.wan_type == WAN_4G) {
            CR_SPAWN2(taskNetwork4G, network_id);
            CR_JOIN(network_id);
        }
#endif

#if WITH_IRIDIUM
        if (UIO.wan_type == WAN_IRIDIUM) {
            CR_SPAWN2(taskNetworkIridium, network_id);
            CR_JOIN(network_id);
        }
#endif

        if (UIO.wan_type == WAN_USB) {
            CR_SPAWN2(taskNetworkUSB, network_id);
            CR_JOIN(network_id);
        }
    }

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

  log_warn("Start slow task");
  delay(5 * 60000); // 5 minutes
  log_warn("End slow task");

  CR_END;
}
