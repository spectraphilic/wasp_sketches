#include "WaspUIO.h"


/**
 * Set RTC time from GPS
 *
 * TODO Use ephemiris, apparently this is required to improve power.
 */

CR_TASK(taskGps)
{
  if (cmdSetTimeGPS(NULL) == 1)
  {
    return CR_TASK_ERROR;
  }

  return CR_TASK_STOP;
}
