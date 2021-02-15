#include "WaspUIO.h"
#include <SDI12.h>


extern WaspSDI12 sdi;

CR_TASK(taskQTPY)
{
    int ttt;
    char *next;

    CR_BEGIN;

    CR_DELAY(4000); // Give 4s for the QT Py to start

    if (UIO.action(1, RUN_QTPY_BME280)) {
        ttt = sdi.measure(5);
        if (ttt < 0) { CR_ERROR; }
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.sendCommand(5, "D0") == NULL) { CR_ERROR; }

        float bme_t = strtod(sdi.buffer+1, &next);
        float bme_p = strtod(next, &next);
        float bme_h = strtod(next, &next);
        ADD_SENSOR(SENSOR_BME_77, bme_t, bme_h, bme_p);
    }

    if (UIO.action(1, RUN_QTPY_SHT31)) {
        ttt = sdi.measure(5, 1);
        if (ttt < 0) { CR_ERROR; }
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.sendCommand(5, "D0") == NULL) { CR_ERROR; }

        float sht_t = strtod(sdi.buffer+1, &next);
        float sht_h = strtod(next, &next);
        ADD_SENSOR(SENSOR_SHT31, sht_t, sht_h);
    }

    if (UIO.action(1, RUN_QTPY_TMP117)) {
        ttt = sdi.measure(5, 2);
        if (ttt < 0) { CR_ERROR; }
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.sendCommand(5, "D0") == NULL) { CR_ERROR; }

        float temp = strtod(sdi.buffer+1, &next);
        ADD_SENSOR(SENSOR_TMP1XX, temp);
    }

    if (UIO.action(1, RUN_QTPY_VL53L1)) {
        ttt = sdi.measure(5, 3);
        if (ttt < 0) { CR_ERROR; }
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.sendCommand(5, "D0") == NULL) { CR_ERROR; }

        int16_t distance = strtol(sdi.buffer+1, &next, 10);
        ADD_SENSOR(SENSOR_VL53L1X, 0, &distance);
    }

    if (UIO.action(1, RUN_QTPY_MLX90614)) {
        ttt = sdi.measure(5, 4);
        if (ttt < 0) { CR_ERROR; }
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.sendCommand(5, "D0") == NULL) { CR_ERROR; }

        float mlx_a = strtod(sdi.buffer+1, &next);
        float mlx_o = strtod(next, &next);
        ADD_SENSOR(SENSOR_MLX90614, mlx_o, mlx_a); // object, ambient
    }

    CR_END;
}
