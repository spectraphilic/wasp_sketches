#include "WaspUIO.h"
#include <SDI12.h>


extern WaspSDI12 sdi;

CR_TASK(taskQTPY)
{
    static int n;
    unsigned int ttt;
    char *next;
    const int address = 5;

    CR_BEGIN;

    CR_DELAY(5000); // Wait 4s for the QT Py to start

    n = sdi.measure(&ttt, address);
    if (n > 0) {
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.data(address) != NULL) {
            next = sdi.buffer + 1;
            uint16_t f1 = strtoul(next, &next, 10);
            uint16_t f2 = strtoul(next, &next, 10);
            uint16_t f3 = strtoul(next, &next, 10);
            uint16_t f4 = strtoul(next, &next, 10);
            uint16_t f5 = strtoul(next, &next, 10);
            uint16_t f6 = strtoul(next, &next, 10);
            uint16_t f7 = strtoul(next, &next, 10);
            uint16_t f8 = strtoul(next, &next, 10);
            uint16_t clear = strtoul(next, &next, 10);
            uint16_t nir = strtoul(next, &next, 10);
            ADD_SENSOR(SENSOR_AS7341, f1, f2, f3, f4, f5, f6, f7, f8, clear, nir);
        }
    }

    n = sdi.measure(&ttt, address, 1);
    if (n > 0) {
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.data(address) != NULL) {
            next = sdi.buffer + 1;
            float bme_t = strtod(next, &next);
            float bme_h = strtod(next, &next);
            float bme_p = strtod(next, &next);
            ADD_SENSOR(SENSOR_BME_77, bme_t, bme_h, bme_p);
        }
    }

/*
    n = sdi.measure(&ttt, address, 2);
    if (n > 0) {
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.data(address) != NULL) {
            next = sdi.buffer + 1;
            float temp = strtod(next, &next);
            float acc_x = strtod(next, &next);
            float acc_y = strtod(next, &next);
            float acc_z = strtod(next, &next);
            float mag_x = strtod(next, &next);
            float mag_y = strtod(next, &next);
            float mag_z = strtod(next, &next);
            float gyro_x = strtod(next, &next);
            float gyro_y = strtod(next, &next);
            float gyro_z = strtod(next, &next);
            ADD_SENSOR(SENSOR_ICM20X, temp, acc_x, acc_y, acc_z, mag_x, mag_y, mag_z, gyro_x, gyro_y, gyro_z);
        }
    }
*/

    n = sdi.measure(&ttt, address, 3);
    if (n > 0) {
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.data(address) != NULL) {
            next = sdi.buffer + 1;
            float mlx_o = strtod(next, &next);
            float mlx_a = strtod(next, &next);
            ADD_SENSOR(SENSOR_MLX90614, mlx_o, mlx_a); // object, ambient
        }
    }

    n = sdi.measure(&ttt, address, 4);
    if (n > 0) {
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.data(address) != NULL) {
            next = sdi.buffer + 1;
            float sht_t = strtod(next, &next);
            float sht_h = strtod(next, &next);
            ADD_SENSOR(SENSOR_SHT31, sht_t, sht_h);
        }
    }

    n = sdi.measure(&ttt, address, 5);
    if (n > 0) {
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.data(address) != NULL) {
            next = sdi.buffer + 1;
            float temp = strtod(next, &next);
            ADD_SENSOR(SENSOR_TMP1XX, temp);
        }
    }

    n = sdi.measure(&ttt, address, 6);
    if (n > 0) {
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.data(address) != NULL) {
            next = sdi.buffer + 1;
            uint16_t prox = strtoul(next, &next, 10);
            uint16_t lux = strtoul(next, &next, 10);
            uint16_t white = strtoul(next, &next, 10);
            ADD_SENSOR(SENSOR_VCNL4040, prox, lux, white);
        }
    }

    n = sdi.measure(&ttt, address, 7);
    if (n > 0) {
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.data(address) != NULL) {
            next = sdi.buffer + 1;
            float lux = strtod(next, &next);
            float white = strtod(next, &next);
            uint16_t als = strtoul(next, &next, 10);
            ADD_SENSOR(SENSOR_VEML7700, lux, white, als);
        }
    }

    n = sdi.measure(&ttt, address, 8);
    if (n > 0) {
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.data(address) != NULL) {
            next = sdi.buffer + 1;
            unsigned int distances[n];
            for (int i = 0; i < n; i++) {
                distances[i] = strtol(next, &next, 10);
            }
            ADD_SENSOR(SENSOR_VL53L1X, n, distances);
        }
    }

    CR_END;
}
