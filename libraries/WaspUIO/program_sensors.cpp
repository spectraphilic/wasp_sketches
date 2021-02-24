#include "WaspUIO.h"


/**
 * Internal sensors
 *
 * The accelerometer uses the I2C bus.
 */

CR_TASK(taskHealthFrame)
{
  // Battery level
  if (UIO.action(1, RUN_BATTERY)) {
    if (UIO.batteryType == BATTERY_LITHIUM) {
      ADD_SENSOR(SENSOR_BAT, UIO.batteryLevel);
    } else {
      ADD_SENSOR(SENSOR_VOLTS, UIO.batteryVolts);
    }
  }

  return CR_TASK_STOP;
}


/**
 * Functions to turn on/off sensor power switches.
 * Supports the SDI-12 bus...
 *
 * The SDI-12 bus uses only 5V.
 */

CR_TASK(taskSensors)
{
#if WITH_I2C || WITH_SDI || WITH_1WIRE || WITH_MB
  static tid_t id;
#endif

  CR_BEGIN;

#if WITH_I2C
  // I2C
  if (UIO.action(10, RUN_ACC, RUN_BME280, RUN_LAGOPUS_AS7263,
                 RUN_LAGOPUS_AS7265, RUN_LAGOPUS_BME280, RUN_LAGOPUS_MLX90614,
                 RUN_LAGOPUS_TMP102, RUN_LAGOPUS_VL53L1X, RUN_TMP117,
                 RUN_SHT31))
  {
    UIO.pwr_i2c(1);
    CR_SPAWN2(taskI2C, id);
    CR_JOIN(id);
    UIO.pwr_i2c(0);
  }
#endif

#if WITH_SDI
  // SDI-12
  if (UIO.action(3, RUN_CTD10, RUN_DS2, RUN_ATMOS))
  {
    UIO.pwr_sdi12(1);
    CR_SPAWN2(taskSdi, id);
    CR_JOIN(id);
    UIO.pwr_sdi12(0);
  }
#endif

#if WITH_1WIRE
  // OneWire
  if (UIO.action(1, RUN_DS1820))
  {
    UIO.pwr_1wire(1);
    CR_SPAWN2(task1Wire, id);
    CR_JOIN(id);
    UIO.pwr_1wire(0);
  }
#endif

#if WITH_MB
  // Maxbotix
  if (UIO.action(1, RUN_MB))
  {
    UIO.pwr_mb(1);
    CR_SPAWN2(taskTTL, id);
    CR_JOIN(id);
    UIO.pwr_mb(0);
  }
#endif

#if WITH_QTPY
  // SDI-12 QT-Py
  if (UIO.action(1, RUN_QTPY)) {
    UIO.pwr_3v3(1);
    CR_SPAWN2(taskQTPY, id);
    CR_JOIN(id);
    UIO.pwr_3v3(0);
  }
#endif

  CR_END;
}


/**
 * OneWire
 */

CR_TASK(task1Wire)
{
  uint8_t max = 99;
  int values[max];

  // TODO
  // - spawn 1 task per pin
  // - give control back with CR_DELAY

  CR_BEGIN;

  // Read
  uint8_t total = UIO.readDS18B20(values, max);

  // Frame(s)
  uint8_t max_n = 30;
  uint8_t done = 0;
  while (done < total)
  {
    uint8_t todo = total - done;
    uint8_t n = (todo <= max_n)? todo: max_n;
    ADD_SENSOR(SENSOR_DS18B20, n, &(values[done]));
    done += n;
  }

  CR_END;
}

/**
 * I2C
 */

CR_TASK(taskI2C)
{
  static tid_t acc_id, bme280_76_id, as7263_id, as7265_id, bme280_id, mlx_id, tmp102_id, vl_id, tmp117_id, sht31_id;
  bool acc = UIO.action(1, RUN_ACC);
  bool bme280_76 = UIO.action(1, RUN_BME280);

  #if WITH_AS7265
  bool as7263 = UIO.action(1, RUN_LAGOPUS_AS7263);
  bool as7265 = UIO.action(1, RUN_LAGOPUS_AS7265);
  #endif

  bool bme280 = UIO.action(1, RUN_LAGOPUS_BME280);
  bool mlx = UIO.action(1, RUN_LAGOPUS_MLX90614);
  bool tmp102 = UIO.action(1, RUN_LAGOPUS_TMP102);
  bool vl = UIO.action(1, RUN_LAGOPUS_VL53L1X);
  bool tmp117 = UIO.action(1, RUN_TMP117);
  bool sht31 = UIO.action(1, RUN_SHT31);

  CR_BEGIN;

  USB.ON();
  if (acc)       { CR_SPAWN2(taskI2C_ACC, acc_id); }
  if (bme280_76) { CR_SPAWN2(taskI2C_BME280_76, bme280_76_id); }

  #if WITH_AS7265
  if (as7263)    { CR_SPAWN2(taskI2C_AS7263, as7263_id); }
  if (as7265)    { CR_SPAWN2(taskI2C_AS7265, as7265_id); }
  #endif

  if (bme280)    { CR_SPAWN2(taskI2C_BME280, bme280_id); }
  if (mlx)       { CR_SPAWN2(taskI2C_MLX90614, mlx_id); }
  if (tmp102)    { CR_SPAWN2(taskI2C_TMP102, tmp102_id); }
  if (vl)        { CR_SPAWN2(taskI2C_VL53L1X, vl_id); }
  if (tmp117)    { CR_SPAWN2(taskI2C_TMP117, tmp117_id); }
  if (sht31)     { CR_SPAWN2(taskI2C_SHT31, sht31_id); }

  if (acc)       { CR_JOIN(acc_id); }
  if (bme280_76) { CR_JOIN(bme280_76_id); }

  #if WITH_AS7265
  if (as7263)    { CR_JOIN(as7263_id); }
  if (as7265)    { CR_JOIN(as7265_id); }
  #endif

  if (bme280)    { CR_JOIN(bme280_id); }
  if (mlx)       { CR_JOIN(mlx_id); }
  if (tmp102)    { CR_JOIN(tmp102_id); }
  if (vl)        { CR_JOIN(vl_id); }
  if (tmp117)    { CR_JOIN(tmp117_id); }
  if (sht31)     { CR_JOIN(sht31_id); }

  CR_END;
}

CR_TASK(taskI2C_ACC)
{
  // Internal, directly attached to the lemming board
  int x, y, z;
  bool err = UIO.i2c_acc(x, y, z);
  if (err) { return CR_TASK_ERROR; }
  ADD_SENSOR(SENSOR_ACC, x, y, z);
  return CR_TASK_STOP;
}

CR_TASK(taskI2C_BME280_76)
{
  // Internal, directly attached to the lemming board
  float temperature, humidity, pressure;
  bool err = UIO.i2c_BME280(temperature, humidity, pressure);
  if (err) { return CR_TASK_ERROR; }
  ADD_SENSOR(SENSOR_BME_76, temperature, humidity, pressure);
  return CR_TASK_STOP;
}

#if WITH_AS7265
CR_TASK(taskI2C_AS7263)
{
  uint8_t temp;
  float r, s, t, u, v, w;
  bool err = UIO.i2c_AS7263(temp, r, s, t, u, v, w);
  if (err) { return CR_TASK_ERROR; }
  // TODO Frame
  return CR_TASK_STOP;
}

CR_TASK(taskI2C_AS7265)
{
  uint8_t temp;
  float A, B, C, D, E, F, G, H, I, J, K, L, R, S, T, U, V, W;
  bool err = UIO.i2c_AS7265(temp, A, B, C, D, E, F, G, H, I, J, K, L, R, S, T, U, V, W);
  if (err) { return CR_TASK_ERROR; }
  // TODO Frame
  return CR_TASK_STOP;
}
#endif

CR_TASK(taskI2C_BME280)
{
  // External (air), installed in the lagopus shield
  float temperature, humidity, pressure;
  bool err = UIO.i2c_BME280(temperature, humidity, pressure, I2C_ADDRESS_LAGOPUS_BME280);
  if (err) { return CR_TASK_ERROR; }
  ADD_SENSOR(SENSOR_BME_77, temperature, humidity, pressure);
  return CR_TASK_STOP;
}

CR_TASK(taskI2C_MLX90614)
{
  float object, ambient;
  bool err = UIO.i2c_MLX90614(object, ambient);
  if (err) { return CR_TASK_ERROR; }
  ADD_SENSOR(SENSOR_MLX90614, object, ambient);
  return CR_TASK_STOP;
}

CR_TASK(taskI2C_TMP102)
{
  float temperature;
  bool err = UIO.i2c_TMP102(temperature);
  if (err) { return CR_TASK_ERROR; }
  ADD_SENSOR(SENSOR_TMP1XX, temperature);
  return CR_TASK_STOP;
}

CR_TASK(taskI2C_TMP117)
{
  double temperature;
  bool err = UIO.i2c_TMP117(temperature);
  if (err) { return CR_TASK_ERROR; }
  ADD_SENSOR(SENSOR_TMP1XX, (float)temperature);
  return CR_TASK_STOP;
}

CR_TASK(taskI2C_VL53L1X)
{
  uint8_t done = 0;
  int distances[VL_SAMPLES];

  CR_BEGIN;
  uint8_t total = UIO.i2c_VL53L1X(distances, VL_SAMPLES);
  while (done < total)
  {
    uint8_t todo = total - done;
    uint8_t n = (todo < VL_SAMPLES)? todo: VL_SAMPLES;
    ADD_SENSOR(SENSOR_VL53L1X, n, &(distances[done]));
    done += n;
  }
  CR_END;

}

CR_TASK(taskI2C_SHT31)
{
  float temperature, humidity;
  bool err = UIO.i2c_SHT31(temperature, humidity);
  if (err) { return CR_TASK_ERROR; }
  ADD_SENSOR(SENSOR_SHT31, temperature, humidity);
  return CR_TASK_STOP;
}



/**
 * TTL Serial.
 *
 * Maxbotix MB7389. Wiring: GND -> GND, V+ -> 3V3, pin5 -> AUX SERIAL 1RX
 */

CR_TASK(taskTTL)
{
  int distances[MB_SAMPLES];
  uint8_t n;

  CR_BEGIN;

  n = UIO.readMaxbotixSerial(distances, MB_SAMPLES);
  if (n > 0) { ADD_SENSOR(SENSOR_MB73XX, n, distances); }

  CR_END;
}
