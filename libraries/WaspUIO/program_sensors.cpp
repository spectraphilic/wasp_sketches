#include "WaspUIO.h"


/**
 * Internal sensors
 *
 * The accelerometer uses the I2C bus.
 */

CR_TASK(taskHealthFrame)
{
  // Battery level
  if (UIO.action(1, RUN_BATTERY))
  {
    if (UIO.batteryType == BATTERY_LITHIUM)
    {
      ADD_SENSOR(SENSOR_BAT, UIO.batteryLevel);
    }
    else
    {
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
  static tid_t sdi_id, one_id, i2c_id, ttl_id, ext_id;
#if WITH_SDI
  bool sdi = UIO.action(2, RUN_CTD10, RUN_DS2);
  bool ext = UIO.action(1, RUN_WS100); // Externally powered
#endif
#if WITH_1WIRE
  bool one = UIO.action(1, RUN_DS1820); // One Wire
#endif
#if WITH_I2C
  bool i2c = UIO.action(7, RUN_BME280, RUN_LAGOPUS_AS7263, RUN_LAGOPUS_AS7265,
                        RUN_LAGOPUS_BME280, RUN_LAGOPUS_MLX90614,
                        RUN_LAGOPUS_TMP102, RUN_LAGOPUS_VL53L1X);
#endif
#if WITH_MB
  bool ttl = UIO.action(1, RUN_MB);
#endif

  CR_BEGIN;

  // Power On
  UIO.saveState();
#if WITH_SDI
  if (sdi) { UIO.pwr_sdi12(1); }
  if (ext) {}
#endif
#if WITH_1WIRE
  if (one) { UIO.pwr_1wire(1); }
#endif
#if WITH_I2C
  if (i2c) { UIO.pwr_i2c(1); }
#endif
#if WITH_MB
  if (ttl) { UIO.pwr_mb(1); }
#endif

  // Wait for power to stabilize
  CR_DELAY(500);

  // Spawn tasks to take measures
#if WITH_SDI
  if (sdi) { CR_SPAWN2(taskSdi, sdi_id); }
  if (ext) { CR_SPAWN2(taskExt, ext_id); }
#endif
#if WITH_1WIRE
  if (one) { CR_SPAWN2(task1Wire, one_id); }
#endif
#if WITH_I2C
  if (i2c) { CR_SPAWN2(taskI2C, i2c_id); }
#endif
#if WITH_MB
  if (ttl) { CR_SPAWN2(taskTTL, ttl_id); }
#endif

  // Wait for tasks to complete
#if WITH_SDI
  if (sdi) { CR_JOIN(sdi_id); }
  if (ext) { CR_JOIN(ext_id); }
#endif
#if WITH_1WIRE
  if (one) { CR_JOIN(one_id); }
#endif
#if WITH_I2C
  if (i2c) { CR_JOIN(i2c_id); }
#endif
#if WITH_MB
  if (ttl) { CR_JOIN(ttl_id); }
#endif

  // Power Off
  UIO.loadState();

  CR_END;
}


/**
 * OneWire
 */

CR_TASK(task1Wire)
{
  uint8_t max = 99;
  int values[max];
  uint8_t n;

  // TODO
  // - spawn 1 task per pin
  // - give control back with CR_DELAY

  CR_BEGIN;

  n = UIO.readDS18B20(values, max);
  if (n > 0) { ADD_SENSOR(SENSOR_DS18B20, n, values); }

  CR_END;
}

/**
 * I2C
 */

CR_TASK(taskI2C)
{
  static tid_t bme280_76_id, as7263_id, as7265_id, bme280_id, mlx_id, tmp_id, vl_id;
  bool bme280_76 = UIO.action(1, RUN_BME280);
  bool as7263 = UIO.action(1, RUN_LAGOPUS_AS7263);
  bool as7265 = UIO.action(1, RUN_LAGOPUS_AS7265);
  bool bme280 = UIO.action(1, RUN_LAGOPUS_BME280);
  bool mlx = UIO.action(1, RUN_LAGOPUS_MLX90614);
  bool tmp = UIO.action(1, RUN_LAGOPUS_TMP102);
  bool vl = UIO.action(1, RUN_LAGOPUS_VL53L1X);

  CR_BEGIN;

  if (bme280_76) { CR_SPAWN2(taskI2C_BME280_76, bme280_76_id); }
  if (as7263)    { CR_SPAWN2(taskI2C_AS7263, as7263_id); }
  if (as7265)    { CR_SPAWN2(taskI2C_AS7265, as7265_id); }
  if (bme280)    { CR_SPAWN2(taskI2C_BME280, bme280_id); }
  if (mlx)       { CR_SPAWN2(taskI2C_MLX90614, mlx_id); }
  if (tmp)       { CR_SPAWN2(taskI2C_TMP102, tmp_id); }
  if (vl)        { CR_SPAWN2(taskI2C_VL53L1X, vl_id); }

  if (bme280_76) { CR_JOIN(bme280_76_id); }
  if (as7263)    { CR_JOIN(as7263_id); }
  if (as7265)    { CR_JOIN(as7265_id); }
  if (bme280)    { CR_JOIN(bme280_id); }
  if (mlx)       { CR_JOIN(mlx_id); }
  if (tmp)       { CR_JOIN(tmp_id); }
  if (vl)        { CR_JOIN(vl_id); }

  CR_END;
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
  // TODO Frame
  return CR_TASK_STOP;
}

CR_TASK(taskI2C_TMP102)
{
  float temperature;
  bool err = UIO.i2c_TMP102(temperature);
  if (err) { return CR_TASK_ERROR; }
  // TODO Frame
  return CR_TASK_STOP;
}

CR_TASK(taskI2C_VL53L1X)
{
  uint16_t distance;
  bool err = UIO.i2c_VL53L1X(distance);
  if (err) { return CR_TASK_ERROR; }
  // TODO Frame
  //ADD_SENSOR(SENSOR_VL, distance);
  return CR_TASK_STOP;
}


/**
 * TTL Serial.
 *
 * Maxbotix MB7389. Wiring: GND -> GND, V+ -> 3V3, pin5 -> AUX SERIAL 1RX
 */

CR_TASK(taskTTL)
{
  uint16_t mean, sd;
  if (UIO.readMaxbotixSerial(mean, sd, 5)) { return CR_TASK_ERROR; }

  ADD_SENSOR(SENSOR_MB73XX, (uint32_t) mean, (uint32_t) sd);

  return CR_TASK_STOP;
}
