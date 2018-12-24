#include "WaspUIO.h"
#include <SDI12.h>


/*
 * API
 */

WaspSDI12 sdi(PIN_SDI12);


const char* WaspUIO::sdi_identify(uint8_t address)
{
    return sdi.identify(address);
}

char WaspUIO::sdi_read_address()
{
    return sdi.read_address();
}

uint8_t WaspUIO::sdi_set_address(char current_address, char new_address)
{
    return sdi.set_address(current_address, new_address);
}

/*
 * Tasks
 */


CR_TASK(taskSdi)
{
  static tid_t tid;

  CR_BEGIN;
  UIO.sdi12(1);

  // XXX There are 2 incompatible strategies to improve this:
  // - Use the Concurrent command
  // - Use service requests

  // CTD-10
  if (UIO.action(1, RUN_CTD10))
  {
    CR_SPAWN2(taskSdiCtd10, tid);
    CR_JOIN(tid);
  }

  // DS-2
  if (UIO.action(1, RUN_DS2))
  {
    CR_SPAWN2(taskSdiDs2, tid);
    CR_JOIN(tid);
  }

  UIO.sdi12(0);
  CR_END;
}

CR_TASK(taskSdiCtd10)
{
  int ttt;

  CR_BEGIN;

  // Send the measure command
  ttt = sdi.measure(0);
  if (ttt < 0) { CR_ERROR; }

  // TODO We could listen every n ms for a "Service Request" from the sensor
  if (ttt > 0) { CR_DELAY(ttt * 1000); }

  // Send the data command
  if (sdi.data(0) == NULL) { CR_ERROR; }

  // Frame. The result looks like 0+167+17.5+103
  char *next;
  double a, b, c;

  a = strtod(sdi.buffer+1, &next);
  b = strtod(next, &next);
  c = strtod(next, &next);
  ADD_SENSOR(SENSOR_CTD10, a, b, c);

  // Success
  CR_END;
}

CR_TASK(taskSdiDs2)
{
  CR_BEGIN;

  // Send the measure command
  if (sdi.sendCommand(1, "M6") == NULL)
  {
    CR_ERROR;
  }

  // XXX In theory we should wait for the time returned by the M command. But
  // tests show it returns 1, probably because 1s is all it needs to return an
  // instantaneous value. But we want averages, so we have to wait >10s.
  CR_DELAY(11000);

  // Wind speed&direction, air temp
  if (sdi.sendCommand(1, "D0") == NULL)
  {
    CR_ERROR;
  }

  // Frame. The result looks like 1+1.04+347+25.2+1.02-0.24+2.05
  char *next;
  double a, b, c, d, e, f;

  a = strtod(sdi.buffer+1, &next);
  b = strtod(next, &next);
  c = strtod(next, &next);
  d = strtod(next, &next);
  e = strtod(next, &next);
  f = strtod(next, &next);
  ADD_SENSOR(SENSOR_DS2, a, b, c, d, e, f);

  CR_END;
}

CR_TASK(taskExt)
{
  static tid_t tid;

  CR_BEGIN;

  // WS100
  if (UIO.action(1, RUN_WS100))
  {
    CR_SPAWN2(taskSdiWS100, tid);
    CR_JOIN(tid);
  }

  CR_END;
}

CR_TASK(taskSdiWS100)
{
  CR_BEGIN;

  uint8_t retries = 3;
  while (retries > 0 && sdi.sendCommand(2, "") == NULL)
  {
    retries--;
    delay(100);
  }
  if (retries == 0)
  {
    CR_ERROR;
  }

  if (sdi.sendCommand(2, "R0") == NULL)
  {
    CR_ERROR;
  }

  // Frame. The result looks like 2+23.5+0.2+3.2+60
  char *next;
  float a = strtod(sdi.buffer+1, &next);
  float b = strtod(next, &next);
  float c = strtod(next, &next);
  uint8_t d = (uint8_t) strtoul(next, &next, 10);
  float e = strtod(next, &next);
  ADD_SENSOR(SENSOR_WS100, a, b, c, d, e);

  CR_END;
}
