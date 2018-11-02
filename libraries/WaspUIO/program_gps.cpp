#include "WaspUIO.h"


/**
 * Set RTC time from GPS
 *
 */

CR_TASK(taskGps)
{
  UIO.gps(true, true);
  return CR_TASK_STOP;
}
