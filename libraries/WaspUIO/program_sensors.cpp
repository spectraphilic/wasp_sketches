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
  bool i2c = UIO.action(1, RUN_BME280);
#endif
#if WITH_MB
  bool ttl = UIO.action(1, RUN_MB);
#endif

  CR_BEGIN;

  // Power On
  UIO.saveState();
#if WITH_SDI
  if (sdi) { UIO.sdi12(1); }
  if (ext) {}
#endif
#if WITH_1WIRE
  if (one) { UIO.onewire(1); }
#endif
#if WITH_I2C
  if (i2c) { UIO.i2c(1); }
#endif
#if WITH_MB
  if (ttl) { UIO.maxbotix(1); }
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

  UIO.onewire(1);
  n = UIO.readDS18B20(values, max);
  UIO.onewire(0);

  if (n > 0) { ADD_SENSOR(SENSOR_DS18B20, n, values); }

  CR_END;
}

/**
 * I2C
 */

CR_TASK(taskI2C)
{
  float temperature, humidity, pressure;

  UIO.i2c(1);
  UIO.i2c_BME280(temperature, humidity, pressure);
  UIO.i2c(0);

  // Frame
  ADD_SENSOR(SENSOR_BME_TC, temperature);
  ADD_SENSOR(SENSOR_BME_HUM, humidity);
  ADD_SENSOR(SENSOR_BME_PRES, pressure);

  return CR_TASK_STOP;
}

/**
 * TTL Serial.
 *
 * Maxbotix MB7389. Wiring: GND -> GND, V+ -> 3V3, pin5 -> AUX SERIAL 1RX
 */

CR_TASK(taskTTL)
{
  uint16_t median, sd;

  UIO.maxbotix(1);
  if (UIO.readMaxbotixSerial(median, sd, 5)) { return CR_TASK_ERROR; }
  UIO.maxbotix(0);

  ADD_SENSOR(SENSOR_MB73XX, (uint32_t) median, (uint32_t) sd);

  return CR_TASK_STOP;
}
