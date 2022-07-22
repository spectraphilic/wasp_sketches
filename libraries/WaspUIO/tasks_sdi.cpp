#include "WaspUIO.h"
#include <SDI12.h>


extern WaspSDI12 sdi;

/*
 * Tasks: CTD-10
 */

CR_TASK(taskSdi)
{
    static tid_t tid;

    CR_BEGIN;

    // XXX There are 2 incompatible strategies to improve this:
    // - Use the Concurrent command
    // - Use service requests

    // CTD-10
    if (UIO.action(1, RUN_CTD10)) {
        CR_SPAWN2(taskSdiCtd10, tid);
        CR_JOIN(tid);
    }

    // DS-2
    if (UIO.action(1, RUN_DS2)) {
        CR_SPAWN2(taskSdiDs2, tid);
        CR_JOIN(tid);
    }

    // ATMOS-22
    if (UIO.action(1, RUN_ATMOS22)) {
        CR_SPAWN2(taskSdiAtmos22, tid);
        CR_JOIN(tid);
    }

    // ATMOS-41
    if (UIO.action(1, RUN_ATMOS41)) {
        CR_SPAWN2(taskSdiAtmos41, tid);
        CR_JOIN(tid);
    }

    CR_END;
}


/*
 * Tasks: CTD-10
 */

CR_TASK(taskSdiCtd10)
{
    static int n;
    unsigned int ttt;
    float values[5];

    CR_BEGIN;

    // Send the measure command
    n = sdi.measure(&ttt, 0);
    if (n < 1) {
        CR_ERROR;
    }

    // TODO We could listen every n ms for a "Service Request" from the sensor
    if (ttt > 0) {
        CR_DELAY(ttt * 1000);
    }

    // Send the data command
    if (sdi.data(values, 0, n) < n) {
        CR_ERROR;
    }

    // Frame. The result looks like 0+167+17.5+103
    double depth, temp, cond;

    depth = values[0];
    temp = values[1];
    cond = values[2];
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
    if (sdi.sendCommand(1, "M6") == NULL) {
        CR_ERROR;
    }

    // XXX In theory we should wait for the time returned by the M command. But
    // tests show it returns 1, probably because 1s is all it needs to return an
    // instantaneous value. But we want averages, so we have to wait >10s.
    CR_DELAY(11000);

    // aD0!
    // Response example: 1+1.04+347+25.2+1.02-0.24+2.05
    if (sdi.sendCommand(1, "D0") == NULL) {
        CR_ERROR;
    }

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


CR_TASK(taskSdiAtmos22)
{
    char *next;

    CR_BEGIN;

    // aM!
    if (sdi.sendCommand(2, "M") == NULL) {
        CR_ERROR;
    }

    // XXX In theory we should wait for the time returned by the M command. But
    // tests show it returns 1, probably because 1s is all it needs to return an
    // instantaneous value. But we want averages, so we have to wait >10s.
    CR_DELAY(11000);

    // aD0: speed, direction and gust
    if (sdi.sendCommand(2, "D0") == NULL) {
        CR_ERROR;
    }
    double speed = strtod(sdi.buffer+1, &next);
    double dir = strtod(next, &next);
    double gust = strtod(next, &next);

    // aD1: temperature
    if (sdi.sendCommand(2, "D1") == NULL) {
        CR_ERROR;
    }
    double temp = strtod(sdi.buffer+1, &next);

    // aM1!
    if (sdi.sendCommand(2, "M1") == NULL) {
        CR_ERROR;
    }
    delay(1); // Don't use CR_DELAY here, otherwise we will lose local vars above
    // aD0: x, y (orientation)
    if (sdi.sendCommand(2, "D0") == NULL) {
        CR_ERROR;
    }
    double x = strtod(sdi.buffer+1, &next);
    double y = strtod(next, &next);

    // Frame
    ADD_SENSOR(SENSOR_ATMOS22,
        (int16_t)round(speed*100),
        (int16_t)round(dir),
        (int16_t)round(gust*100),
        (int16_t)round(temp*10),
        (int16_t)round(x*10),
        (int16_t)round(y*10)
    );

    CR_END;
}


CR_TASK(taskSdiAtmos41)
{
    char *next;

    if (UIO.nloops == 1) {
        sdi.sendCommand(2, ""); // 2! Acknowledge Active
        return CR_TASK_STOP;
    }

    CR_BEGIN;

    // aR7!
    if (sdi.sendCommand(2, "R7") == NULL) {
        CR_ERROR;
    }

    double solar = strtod(sdi.buffer+1, &next);
    double precipitation = strtod(next, &next);
    double strikes = strtod(next, &next);
    double strikeDistance = strtod(next, &next);
    double windSpeed = strtod(next, &next);
    double windDirection = strtod(next, &next);
    double gustWindSpeed = strtod(next, &next);
    double airTemperature = strtod(next, &next);
    double vaporPressure = strtod(next, &next);
    double atmosphericPressure = strtod(next, &next);
    double relativeHumidity = strtod(next, &next);
    double humiditySensorTemperature = strtod(next, &next);
    double xOrientation = strtod(next, &next);
    double yOrientation = strtod(next, &next);

    // Frame
    ADD_SENSOR(SENSOR_ATMOS41,
        (int16_t)round(solar),
        (int16_t)round(precipitation * 1000),
        (int16_t)round(strikes),
        (int16_t)round(strikeDistance),
        (int16_t)round(windSpeed * 100),
        (int16_t)round(windDirection * 10),
        (int16_t)round(gustWindSpeed * 100),
        (int16_t)round(airTemperature * 10),
        (int16_t)round(vaporPressure * 100),
        (int16_t)round(atmosphericPressure * 100),
        (int16_t)round(relativeHumidity * 1000),
        (int16_t)round(humiditySensorTemperature * 10),
        (int16_t)round(xOrientation * 10),
        (int16_t)round(yOrientation * 10)
    );

    CR_END;
}
