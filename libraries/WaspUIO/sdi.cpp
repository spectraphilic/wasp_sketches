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

uint8_t WaspUIO::sdi_set_address(uint8_t current_address, uint8_t new_address)
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

  // ATMOS
  if (UIO.action(1, RUN_ATMOS))
  {
    CR_SPAWN2(taskSdiAtmos, tid);
    CR_JOIN(tid);
  }

  CR_END;
}


/*
 * Tasks: CTD-10
 */

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
  double depth, temp, cond;

  depth = strtod(sdi.buffer+1, &next);
  temp = strtod(next, &next);
  cond = strtod(next, &next);
  ADD_SENSOR(SENSOR_CTD10,
        (int16_t)round(depth),
        (int16_t)round(temp*10),
        (int32_t)round(cond)
  );

  // Success
  CR_END;
}


/*
 * Tasks: DS-2
 */

CR_TASK(taskSdiDs2)
{
  char *next;

  CR_BEGIN;

  // aM6!
  if (sdi.sendCommand(1, "M6") == NULL) { CR_ERROR; }

  // XXX In theory we should wait for the time returned by the M command. But
  // tests show it returns 1, probably because 1s is all it needs to return an
  // instantaneous value. But we want averages, so we have to wait >10s.
  CR_DELAY(11000);

  // aD0!
  // Response example: 1+1.04+347+25.2+1.02-0.24+2.05
  if (sdi.sendCommand(1, "D0") == NULL) { CR_ERROR; }

  double speed = strtod(sdi.buffer+1, &next);
  double dir = strtod(next, &next);
  double temp = strtod(next, &next);
  double meridional = strtod(next, &next);
  double zonal = strtod(next, &next);
  double gust = strtod(next, &next);

  // Frame
  ADD_SENSOR(SENSOR_DS2,
    (int16_t)round(speed*100),
    (int16_t)round(dir),
    (int16_t)round(temp*10),
    (int16_t)round(meridional*100),
    (int16_t)round(zonal*100),
    (int16_t)round(gust*100)
  );

  CR_END;
}


CR_TASK(taskSdiAtmos)
{
  char *next;

  CR_BEGIN;

  // aM!
  if (sdi.sendCommand(2, "M") == NULL) { CR_ERROR; }

  // XXX In theory we should wait for the time returned by the M command. But
  // tests show it returns 1, probably because 1s is all it needs to return an
  // instantaneous value. But we want averages, so we have to wait >10s.
  CR_DELAY(11000);

  // aD0: speed, direction and gust
  if (sdi.sendCommand(2, "D0") == NULL) { CR_ERROR; }
  double speed = strtod(sdi.buffer+1, &next);
  double dir = strtod(next, &next);
  double gust = strtod(next, &next);

  // aD1: temperature
  if (sdi.sendCommand(2, "D1") == NULL) { CR_ERROR; }
  double temp = strtod(sdi.buffer+1, &next);

  // aM1!
  if (sdi.sendCommand(2, "M1") == NULL) { CR_ERROR; }
  delay(1); // Don't use CR_DELAY here, otherwise we will lose local vars above
  // aD0: x, y (orientation)
  if (sdi.sendCommand(2, "D0") == NULL) { CR_ERROR; }
  double x = strtod(sdi.buffer+1, &next);
  double y = strtod(next, &next);

  // Frame
  ADD_SENSOR(SENSOR_ATMOS,
    (int16_t)round(speed*100),
    (int16_t)round(dir),
    (int16_t)round(gust*100),
    (int16_t)round(temp*10),
    (int16_t)round(x*10),
    (int16_t)round(y*10)
  );

  CR_END;
}


/*
 * Tasks: WS100
 */

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
