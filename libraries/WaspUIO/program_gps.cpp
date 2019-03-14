#include "WaspUIO.h"


/**
 * Get location and set RTC time from GPS
 */

CR_TASK(taskGPS)
{
  UIO.gps(true, true);
  return CR_TASK_STOP;
}

#if WITH_4G
/**
 * Get location
 */

CR_TASK(taskGPS4G)
{
  UIO._4GGPS();
  return CR_TASK_STOP;
}
#endif
