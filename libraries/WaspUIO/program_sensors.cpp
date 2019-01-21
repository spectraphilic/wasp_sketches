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
  static tid_t id;

  CR_BEGIN;

  // I2C
#if WITH_I2C
  if (UIO.action(7, RUN_BME280, RUN_LAGOPUS_AS7263, RUN_LAGOPUS_AS7265,
		 RUN_LAGOPUS_BME280, RUN_LAGOPUS_MLX90614, RUN_LAGOPUS_TMP102,
		 RUN_LAGOPUS_VL53L1X))
  {
    UIO.pwr_i2c(1);
    CR_DELAY(100);
    CR_SPAWN2(taskI2C, id);
    CR_JOIN(id);
    UIO.pwr_i2c(0);
  }
#endif

  // SDI-12
#if WITH_SDI
  if (UIO.action(2, RUN_CTD10, RUN_DS2))
  {
    UIO.pwr_sdi12(1);
    CR_DELAY(500); // FIXME Reduce as much as possible
    CR_SPAWN2(taskSdi, id);
    CR_JOIN(id);
    UIO.pwr_sdi12(0);
  }
  if (UIO.action(1, RUN_WS100)) // Externally powered
  {
    CR_SPAWN2(taskExt, id);
    CR_JOIN(id);
  }
#endif

  // OneWire
#if WITH_1WIRE
  if (UIO.action(1, RUN_DS1820))
  {
    UIO.pwr_1wire(1);
    CR_DELAY(500); // FIXME Reduce as much as possible
    CR_SPAWN2(task1Wire, id);
    CR_JOIN(id);
    UIO.pwr_1wire(0);
  }
#endif

  // Maxbotix
#if WITH_MB
  if (UIO.action(1, RUN_MB))
  {
    UIO.pwr_mb(1);
    CR_DELAY(500); // FIXME Reduce as much as possible
    CR_SPAWN2(taskTTL, id);
    CR_JOIN(id);
    UIO.pwr_mb(0);
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
