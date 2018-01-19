#include "WaspUIO.h"


/**
 * Internal sensors
 *
 * The accelerometer uses the I2C bus.
 */

CR_TASK(taskAcc)
{
  int16_t accX, accY, accZ;

  // ON
  ACC.ON();

  // Check correct operation
  if (ACC.check() != 0x32)
  {
    error(F("acc: check failed"));
    return CR_TASK_ERROR;
  }

  // Read values
  accX = ACC.getX();
  accY = ACC.getY();
  accZ = ACC.getZ();

  // OFF
  ACC.OFF();

  // Frame
  ADD_SENSOR(SENSOR_ACC, accX, accY, accZ);

  return CR_TASK_STOP;
}

CR_TASK(taskHealthFrame)
{
  // Battery level (not for lead acid)
  // TODO Define how often in the menu with action_battery
  if (UIO.batteryType != 2)
  {
    ADD_SENSOR(SENSOR_BAT, UIO.batteryLevel);
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
 * Supports the Agr board and the SDI-12 bus.
 *
 * The Agr board uses both 5V and 3V3. While the SDI-12 bus uses only 5V.
 */

CR_TASK(taskSensors)
{
#if USE_AGR
  bool agr = UIO.action(3, RUN_PRESSURE, RUN_LEAFWETNESS, RUN_SENSIRION);
#endif
#if USE_SDI
  bool sdi = UIO.action(2, RUN_CTD10, RUN_DS2);
#endif
#if USE_I2C
  bool onewire = UIO.action(1, RUN_DS1820);
  bool i2c = UIO.action(1, RUN_BME280);
  bool ttl = UIO.action(1, RUN_MB);
#endif
  static tid_t agr_id, sdi_id, onewire_id, i2c_id, ttl_id;

  CR_BEGIN;

  // Power On
#if USE_AGR
  if (agr)
  {
    info(F("Agr board ON"));
    SensorAgrv20.ON();
  }
#endif
#if USE_SDI
  if (! (WaspRegister & REG_5V) && sdi)
  {
    info(F("5V ON"));
    PWR.setSensorPower(SENS_5V, SENS_ON);
  }
#endif
#if USE_I2C
  if (! (WaspRegister & REG_3V3) && (onewire || i2c || ttl))
  {
    info(F("3V3 ON"));
    PWR.setSensorPower(SENS_3V3, SENS_ON);
  }

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
#endif

  // Wait for power to stabilize
  CR_DELAY(500);

  // Spawn tasks to take measures
#if USE_AGR
  if (agr)     { CR_SPAWN2(taskAgr, agr_id); }
#endif
#if USE_SDI
  if (sdi)     { CR_SPAWN2(taskSdi, sdi_id); }
#endif
#if USE_I2C
  if (onewire) { CR_SPAWN2(task1Wire, onewire_id); }
  if (i2c)     { CR_SPAWN2(taskI2C, i2c_id); }
  if (ttl)     { CR_SPAWN2(taskTTL, ttl_id); }
#endif

  // Wait for tasks to complete
#if USE_AGR
  if (agr)     { CR_JOIN(agr_id); }
#endif
#if USE_SDI
  if (sdi)     { CR_JOIN(sdi_id); }
#endif
#if USE_I2C
  if (onewire) { CR_JOIN(onewire_id); }
  if (i2c)     { CR_JOIN(i2c_id); }
  if (ttl)     { CR_JOIN(ttl_id); }
#endif

  // Power Off
#if USE_AGR
  if (agr)
  {
    info(F("Agr board OFF"));
    SensorAgrv20.OFF();
  }
#endif
  if (WaspRegister & REG_5V)
  {
    info(F("5V OFF"));
    PWR.setSensorPower(SENS_5V, SENS_OFF);
  }
  if (WaspRegister & REG_3V3)
  {
    info(F("3V3 OFF"));
    PWR.setSensorPower(SENS_3V3, SENS_OFF);
  }

  CR_END;
}

/**
 * The Agr board
 */

CR_TASK(taskAgr)
{
  static tid_t p_id, lc_id;

  CR_BEGIN;

  // Measure
  if (UIO.action(1, RUN_PRESSURE))
  {
    CR_SPAWN2(taskAgrPressure, p_id);
  }
  if (UIO.action(2, RUN_LEAFWETNESS, RUN_SENSIRION))
  {
    CR_SPAWN2(taskAgrLC, lc_id);
  }

  // Wait
  if (UIO.action(1, RUN_PRESSURE))
  {
    CR_JOIN(p_id);
  }
  if (UIO.action(2, RUN_LEAFWETNESS, RUN_SENSIRION))
  {
    CR_JOIN(lc_id);
  }

  CR_END;
}

#if USE_AGR
CR_TASK(taskAgrPressure)
{
  float pressure;

  CR_BEGIN;

  UIO.on(UIO_PRESSURE);
  CR_DELAY(50);
  pressure = SensorAgrv20.readValue(SENS_AGR_PRESSURE); // Read
  UIO.off(UIO_PRESSURE);
  ADD_SENSOR(SENSOR_PA, pressure);

  CR_END;
}

/* The Low consumption group. */
CR_TASK(taskAgrLC)
{
  float temperature, humidity, wetness;

  CR_BEGIN;

  // Leaf wetness
  if (UIO.action(1, RUN_LEAFWETNESS))
  {
    UIO.on(UIO_LEAFWETNESS);
    CR_DELAY(50);
    wetness = SensorAgrv20.readValue(SENS_AGR_LEAF_WETNESS);
    ADD_SENSOR(SENSOR_LW, wetness);
  }

  // Sensirion (temperature, humidity)
  if (UIO.action(1, RUN_SENSIRION))
  {
    UIO.on(UIO_SENSIRION);
    CR_DELAY(50);
    temperature = SensorAgrv20.readValue(SENS_AGR_SENSIRION, SENSIRION_TEMP);
    humidity = SensorAgrv20.readValue(SENS_AGR_SENSIRION, SENSIRION_HUM);
    ADD_SENSOR(SENSOR_TCB, temperature); // Add digital temperature
    ADD_SENSOR(SENSOR_HUMB, humidity); // Add digital humidity
  }

  // OFF
  if (UIO.action(1, RUN_LEAFWETNESS))
  {
    UIO.off(UIO_LEAFWETNESS);
  }
  if (UIO.action(1, RUN_SENSIRION))
  {
    UIO.off(UIO_SENSIRION);
  }

  CR_END;
}
#endif


/**
 * SDI-12
 */

CR_TASK(taskSdi)
{
  static tid_t tid;

  CR_BEGIN;

  UIO.on(UIO_SDI12);

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

  UIO.off(UIO_SDI12);

  CR_END;
}

CR_TASK(taskSdiCtd10)
{
  int ttt;

  CR_BEGIN;

  // Send the measure command
  ttt = mySDI12.measure(0);
  if (ttt < 0)
  {
    CR_ERROR;
  }

  if (ttt > 0)
  {
    // TODO We could listen every n ms for a "Service Request" from the sensor
    CR_DELAY(ttt * 1000);
  }

  // Send the data command
  if (mySDI12.data(0))
  {
    CR_ERROR;
  }

  // Frame. The result looks like 0+167+17.5+103
  char *next;
  double a, b, c;

  a = strtod(mySDI12.buffer+1, &next);
  b = strtod(next, &next);
  c = strtod(next, &next);
  ADD_SENSOR(SENSOR_SDI12_CTD10, a, b, c);

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
  ADD_SENSOR(SENSOR_SDI12_DS2_1, a, b, c);

  a = strtod(next, &next);
  b = strtod(next, &next);
  c = strtod(next, &next);
  ADD_SENSOR(SENSOR_SDI12_DS2_2, a, b, c);

  CR_END;
}

/**
 * OneWire
 */

CR_TASK(task1Wire)
{
  uint8_t addr[8];
  uint8_t data[12];
  char addr_str[17];
  char data_str[17];
  uint8_t present, crc, n;
  int16_t temp;
  float temp_f;
  char temp_str[20];
  WaspOneWire oneWire(PIN_1WIRE);

  CR_BEGIN;

  // For now we only support the DS1820, so here just read that directly
  // We assume we have a chain of DS1820 sensors, and read all of them.

  // delay needed for DS18B20 to response
  // CR_DELAY(750); // This dosn't work... it only read sensor when it tries to send frames
  delay(750);       // ...but this does! Check this???

  present = oneWire.reset();
  if (! present)
  {
    error(F("OneWire no devices attached"));
    CR_ERROR;
  }
  UIO.on(UIO_1WIRE);

  // Send conversion command to all sensors
  oneWire.skip();
  oneWire.write(0x44, 1); // Keep sensors powered (parasite mode)
  CR_DELAY(1000);         // 750ms may be enough

  // TODO It may be better to search once in the setup, or in the menu, and
  // store the addresses in the EEPROM
  n = 0;
  oneWire.reset_search();
  while (oneWire.search(addr))
  {
    Utils.hex2str(addr, addr_str, 8);
    // Check address CRC
    crc = oneWire.crc8(addr, 7);
    if (crc != addr[7])
    {
      error(F("OneWire %s bad address, CRC failed: %02X"), addr_str, crc);
      break;
    }

    // Check device type
    if (addr[0] == 0x28) // DS18B20
    {
      n++;
    }
    else
    {
      warn(F("OneWire %0s unexpected device type: %02X"), addr_str, addr[0]);
      continue;
    }

    // Read value
    present = oneWire.reset();
    oneWire.select(addr);
    oneWire.write(0xBE); // Read Scratchpad
    oneWire.read_bytes(data, 9); // We need 9 bytes

    crc = oneWire.crc8(data, 8);
    if (crc != data[8])
    {
      Utils.hex2str(data, data_str, 8);
      warn(F("OneWire %s bad data, CRC failed: %s %02X"), addr_str, data_str, crc);
    }

    // Convert to float. Formula for the DS18B20 model.
    temp = (data[1] << 8) | data[0];
    temp_f = (float) temp / 16;
    ADD_SENSOR(SENSOR_DS1820, temp_f);

    // Debug
    Utils.float2String(temp_f, temp_str, 2);
    debug(F("OneWire %s : %s"), addr_str, temp_str);
  }

  debug(F("OneWire %d devices measured"), n);
  oneWire.depower();
  UIO.off(UIO_1WIRE);

  CR_END;
}

/**
 * I2C
 */

CR_TASK(taskI2C)
{
  float temperature, humidity, pressure;
  char aux[20];

  UIO.on(UIO_I2C);

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

  UIO.off(UIO_I2C);

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

  if (UIO.readMaxbotixSerial(median, sd, 5))
  {
    return CR_TASK_ERROR;
  }
  ADD_SENSOR(SENSOR_MB73XX, (uint32_t) median, (uint32_t) sd);

  return CR_TASK_STOP;
}
