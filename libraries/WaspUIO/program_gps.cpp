#include "WaspUIO.h"


/**
 * Set RTC time from GPS
 *
 * TODO Use ephemiris, apparently this is required to improve power.
 */

CR_TASK(taskGps)
{
  cmdTimeGPS(NULL);
  return CR_TASK_STOP;
}
