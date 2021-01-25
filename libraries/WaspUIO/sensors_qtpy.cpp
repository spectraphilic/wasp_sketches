#include "WaspUIO.h"
#include <SDI12.h>


extern WaspSDI12 sdi;

CR_TASK(taskQTPY)
{
    static tid_t tid;

    CR_BEGIN;

    if (UIO.action(1, RUN_QTPY_BME280)) {
        CR_SPAWN2(taskQTPY_BME280, tid);
        CR_JOIN(tid);
    }

    if (UIO.action(1, RUN_QTPY_SHT31)) {
        CR_SPAWN2(taskQTPY_SHT31, tid);
        CR_JOIN(tid);
    }

    if (UIO.action(1, RUN_QTPY_TMP117)) {
        CR_SPAWN2(taskQTPY_TMP117, tid);
        CR_JOIN(tid);
    }

    CR_END;
}

CR_TASK(taskQTPY_BME280)
{
    int ttt;
    char *next;

    CR_BEGIN;

    ttt = sdi.measure(5);
    if (ttt < 0) { CR_ERROR; }
    if (ttt > 0) { CR_DELAY(ttt * 1000); }
    if (sdi.sendCommand(5, "D0") == NULL) { CR_ERROR; }

    float bme_t = strtod(sdi.buffer+1, &next);
    float bme_p = strtod(next, &next);
    float bme_h = strtod(next, &next);
    ADD_SENSOR(SENSOR_BME_77, bme_t, bme_h, bme_p);

    CR_END;
}

CR_TASK(taskQTPY_SHT31)
{
    int ttt;
    char *next;

    CR_BEGIN;

    ttt = sdi.measure(5, 1);
    if (ttt < 0) { CR_ERROR; }
    if (ttt > 0) { CR_DELAY(ttt * 1000); }
    if (sdi.sendCommand(5, "D1") == NULL) { CR_ERROR; }

    float sht_t = strtod(sdi.buffer+1, &next);
    float sht_h = strtod(next, &next);
    ADD_SENSOR(SENSOR_SHT31, sht_t, sht_h);

    CR_END;
}

CR_TASK(taskQTPY_TMP117)
{
    int ttt;
    char *next;

    CR_BEGIN;

    ttt = sdi.measure(5, 2);
    if (ttt < 0) { CR_ERROR; }
    if (ttt > 0) { CR_DELAY(ttt * 1000); }
    if (sdi.sendCommand(5, "D2") == NULL) { CR_ERROR; }

    float temp = strtod(sdi.buffer+1, &next);
    ADD_SENSOR(SENSOR_TMP1XX, temp);

    CR_END;
}
