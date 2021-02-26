#include "WaspUIO.h"
#include <SDI12.h>


extern WaspSDI12 sdi;

CR_TASK(taskQTPY)
{
    static int n;
    unsigned int ttt;
    const int address = 5;
    float values[30];

    CR_BEGIN;

    CR_DELAY(5000); // Wait 4s for the QT Py to start

    n = sdi.measure(&ttt, address);
    if (n > 0) {
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.data(values, address, n) == n) {
            uint16_t f1 = (uint16_t) values[0];
            uint16_t f2 = (uint16_t) values[1];
            uint16_t f3 = (uint16_t) values[2];
            uint16_t f4 = (uint16_t) values[3];
            uint16_t f5 = (uint16_t) values[4];
            uint16_t f6 = (uint16_t) values[5];
            uint16_t f7 = (uint16_t) values[6];
            uint16_t f8 = (uint16_t) values[7];
            uint16_t clear = (uint16_t) values[8];
            uint16_t nir = (uint16_t) values[9];
            ADD_SENSOR(SENSOR_AS7341, f1, f2, f3, f4, f5, f6, f7, f8, clear, nir);
        }
    }

    n = sdi.measure(&ttt, address, 1);
    if (n > 0) {
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.data(values, address, n) == n) {
            float bme_t = values[0];
            float bme_h = values[1];
            float bme_p = values[2];
            ADD_SENSOR(SENSOR_BME_77, bme_t, bme_h, bme_p);
        }
    }

/*
    n = sdi.measure(&ttt, address, 2);
    if (n > 0) {
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.data(values, address, n) == n) {
            float temp = values[0];
            float acc_x = values[1];
            float acc_y = values[2];
            float acc_z = values[3];
            float mag_x = values[4];
            float mag_y = values[5];
            float mag_z = values[6];
            float gyro_x = values[7];
            float gyro_y = values[8];
            float gyro_z = values[9];
            ADD_SENSOR(SENSOR_ICM20X, temp, acc_x, acc_y, acc_z, mag_x, mag_y, mag_z, gyro_x, gyro_y, gyro_z);
        }
    }
*/

    n = sdi.measure(&ttt, address, 3);
    if (n > 0) {
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.data(values, address, n) == n) {
            float mlx_o = values[0];
            float mlx_a = values[1];
            ADD_SENSOR(SENSOR_MLX90614, mlx_o, mlx_a); // object, ambient
        }
    }

    n = sdi.measure(&ttt, address, 4);
    if (n > 0) {
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.data(values, address, n) == n) {
            float sht_t = values[0];
            float sht_h = values[1];
            ADD_SENSOR(SENSOR_SHT31, sht_t, sht_h);
        }
    }

    n = sdi.measure(&ttt, address, 5);
    if (n > 0) {
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.data(values, address, n) == n) {
            float temp = values[0];
            ADD_SENSOR(SENSOR_TMP1XX, temp);
        }
    }

    n = sdi.measure(&ttt, address, 6);
    if (n > 0) {
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.data(values, address, n) == n) {
            uint16_t prox = (uint16_t) values[0];
            uint16_t lux = (uint16_t) values[1];
            uint16_t white = (uint16_t) values[2];
            ADD_SENSOR(SENSOR_VCNL4040, prox, lux, white);
        }
    }

    n = sdi.measure(&ttt, address, 7);
    if (n > 0) {
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.data(values, address, n) == n) {
            float lux = values[0];
            float white = values[1];
            uint16_t als = (uint16_t) values[2];
            ADD_SENSOR(SENSOR_VEML7700, lux, white, als);
        }
    }

    n = sdi.measure(&ttt, address, 8);
    if (n > 0) {
        if (ttt > 0) { CR_DELAY(ttt * 1000); }
        if (sdi.data(values, address, n) == n) {
            unsigned int distances[n];
            for (int i = 0; i < n; i++) {
                distances[i] = (unsigned int) values[i];
            }
            ADD_SENSOR(SENSOR_VL53L1X, n, distances);
        }
    }

    CR_END;
}
