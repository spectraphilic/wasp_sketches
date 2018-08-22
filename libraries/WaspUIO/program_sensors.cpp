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

  // RTC temperature (v12 only)
  if (_boot_version < 'H')
  {
    ADD_SENSOR(SENSOR_IN_TEMP, UIO.rtc_temp); // RTC temperature in Celsius (float)
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
  bool sdi = UIO.action(2, RUN_CTD10, RUN_DS2);
  bool one = UIO.action(1, RUN_DS1820); // One Wire
  bool i2c = UIO.action(1, RUN_BME280);
  bool ttl = UIO.action(1, RUN_MB);
  bool ext = UIO.action(1, RUN_WS100); // Externally powered devices
  static tid_t sdi_id, one_id, i2c_id, ttl_id, ext_id;

  CR_BEGIN;

  // Power On
  UIO.saveState();
  if (sdi) { UIO.sdi12(1); }
  if (one) { UIO.onewire(1); }
  if (i2c) { UIO.i2c(1); }
  if (ttl) { UIO.maxbotix(1); }
  if (ext) {}

  // Init BME-280. Copied from BME280::ON to avoid the 100ms delay
  // TODO Do this once in the setup
  if (UIO.action(1, RUN_BME280))
  {
    // Check if the sensor is accesible
    if (BME.checkID() == 1)
    {
      // Read the calibration registers
      BME.readCalibration();
    }
  }

  // Wait for power to stabilize
  CR_DELAY(500);

  // Spawn tasks to take measures
  if (sdi) { CR_SPAWN2(taskSdi, sdi_id); }
  if (one) { CR_SPAWN2(task1Wire, one_id); }
  if (i2c) { CR_SPAWN2(taskI2C, i2c_id); }
  if (ttl) { CR_SPAWN2(taskTTL, ttl_id); }
  if (ext) { CR_SPAWN2(taskExt, ext_id); }

  // Wait for tasks to complete
  if (sdi) { CR_JOIN(sdi_id); }
  if (one) { CR_JOIN(one_id); }
  if (i2c) { CR_JOIN(i2c_id); }
  if (ttl) { CR_JOIN(ttl_id); }
  if (ext) { CR_JOIN(ext_id); }

  // Power Off
  UIO.loadState();

  CR_END;
}


/**
 * SDI-12
 */

CR_TASK(taskSdi)
{
  static tid_t tid;

  CR_BEGIN;
  UIO.sdi12(1);

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

  UIO.sdi12(0);
  CR_END;
}

CR_TASK(taskSdiCtd10)
{
  int ttt;

  CR_BEGIN;

  // Send the measure command
  ttt = mySDI12.measure(0);
  if (ttt < 0) { CR_ERROR; }

  // TODO We could listen every n ms for a "Service Request" from the sensor
  if (ttt > 0) { CR_DELAY(ttt * 1000); }

  // Send the data command
  if (mySDI12.data(0)) { CR_ERROR; }

  // Frame. The result looks like 0+167+17.5+103
  char *next;
  double a, b, c;

  a = strtod(mySDI12.buffer+1, &next);
  b = strtod(next, &next);
  c = strtod(next, &next);
  ADD_SENSOR(SENSOR_CTD10, a, b, c);

  // Success
  CR_END;
}

CR_TASK(taskSdiDs2)
{
  CR_BEGIN;

  // Send the measure command
  if (mySDI12.command2address(1, "M6"))
  {
    CR_ERROR;
  }

  // XXX In theory we should wait for the time returned by the M command. But
  // tests show it returns 1, probably because 1s is all it needs to return an
  // instantaneous value. But we want averages, so we have to wait >10s.
  CR_DELAY(11000);

  // Wind speed&direction, air temp
  if (mySDI12.command2address(1, "D0"))
  {
    CR_ERROR;
  }

  // Frame. The result looks like 1+1.04+347+25.2+1.02-0.24+2.05
  char *next;
  double a, b, c;

  a = strtod(mySDI12.buffer+1, &next);
  b = strtod(next, &next);
  c = strtod(next, &next);
  ADD_SENSOR(SENSOR_DS2_1, a, b, c);

  a = strtod(next, &next);
  b = strtod(next, &next);
  c = strtod(next, &next);
  ADD_SENSOR(SENSOR_DS2_2, a, b, c);

  CR_END;
}

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
  int ttt;

  CR_BEGIN;

/*
  if (mySDI12.command2address(2, ""))
  {
    if (mySDI12.command2address(2, ""))
    {
      CR_ERROR;
    }
  }
*/

  if (mySDI12.command2address(2, "R0"))
  {
    CR_ERROR;
  }

  // Frame. The result looks like 2+23.5+0.2+3.2+60
  char *next;
  float a = strtod(mySDI12.buffer+1, &next);
  float b = strtod(next, &next);
  float c = strtod(next, &next);
  uint8_t d = (uint8_t) strtoul(next, &next, 10);
  float e = strtod(next, &next);
  ADD_SENSOR(SENSOR_WS100, a, b, c, d, e);

  CR_END;
}

/**
 * OneWire
 */

CR_TASK(task1Wire)
{
  uint8_t max = 40;
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
  char aux[20];

  UIO.i2c(1);

  // Read enviromental variables
  temperature = BME.getTemperature(BME280_OVERSAMP_1X, 0);
  humidity = BME.getHumidity(BME280_OVERSAMP_1X);
  pressure = BME.getPressure(BME280_OVERSAMP_1X, 0);

  // Debug
  Utils.float2String(temperature, aux, 2);
  debug(F("BME-280 Temperature: %s Celsius Degrees"), aux);

  Utils.float2String(humidity, aux, 2);
  debug(F("BME-280 Humidity   : %s %%RH"), aux);

  Utils.float2String(pressure, aux, 2);
  debug(F("BME-280 Pressure   : %s Pa"), aux);

  // Frame
  ADD_SENSOR(SENSOR_BME_TC, temperature);
  ADD_SENSOR(SENSOR_BME_HUM, humidity);
  ADD_SENSOR(SENSOR_BME_PRES, pressure);

  UIO.i2c(0);
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
