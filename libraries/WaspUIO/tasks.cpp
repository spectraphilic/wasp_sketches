#include "WaspUIO.h"


/**
 * Main task
 */

CR_TASK(taskMain)
{
  static tid_t health_id, sensors_id, network_id, gps_id;

  CR_BEGIN;

  // Create the first frame
  UIO.createFrame(true);

  // Sensors
  CR_SPAWN2(taskHealthFrame, health_id);
  CR_SPAWN2(taskSensors, sensors_id);

  // Network
  if ((UIO.flags & FLAG_NETWORK) && UIO.action(1, 12)) // 12 x 5min = 1hour
  {
    CR_SPAWN2(taskNetwork, network_id);
    CR_JOIN(network_id);
  }

  CR_JOIN(health_id);
  CR_JOIN(sensors_id);

  // GPS (Once a day)
  // The RTC is DS3231SN (v12) or DS1337C (v15), its accuracy is not enough
  // for our networking requirements, so we have to sync it once a day. See
  // http://hycamp.org/private-area/waspmote-rtc/
  if (UIO.hasGPS && UIO.time.minute == 0 && UIO.time.hour == 0)
  {
    CR_SPAWN2(taskGps, gps_id);
    CR_JOIN(gps_id);
  }

  // Save the last frame, if there is something to save
  if (frame.numFields > 1)
  {
    UIO.frame2Sd();
  }

  //CR_SPAWN(taskSlow);

  CR_END;
}


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
  // Battery
  ADD_SENSOR(SENSOR_BAT, UIO.batteryLevel); // Battery level (uint8_t)

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
  bool agr = UIO.action(3, UIO.sensor_pressure, UIO.sensor_leafwetness, UIO.sensor_sensirion);
#endif
#if USE_SDI
  bool sdi = UIO.action(2, UIO.sensor_ctd10, UIO.sensor_ds2);
#endif
#if USE_I2C
  bool onewire = UIO.action(1, UIO.sensor_ds1820);
  bool i2c = UIO.action(1, UIO.sensor_bme280);
  bool ttl = UIO.action(1, UIO.sensor_mb);
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
  if (UIO.action(1, UIO.sensor_bme280))
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
  if (UIO.action(1, UIO.sensor_pressure))
  {
    CR_SPAWN2(taskAgrPressure, p_id);
  }
  if (UIO.action(2, UIO.sensor_leafwetness, UIO.sensor_sensirion))
  {
    CR_SPAWN2(taskAgrLC, lc_id);
  }

  // Wait
  if (UIO.action(1, UIO.sensor_pressure))
  {
    CR_JOIN(p_id);
  }
  if (UIO.action(2, UIO.sensor_leafwetness, UIO.sensor_sensirion))
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
  if (UIO.action(1, UIO.sensor_leafwetness))
  {
    UIO.on(UIO_LEAFWETNESS);
    CR_DELAY(50);
    wetness = SensorAgrv20.readValue(SENS_AGR_LEAF_WETNESS);
    ADD_SENSOR(SENSOR_LW, wetness);
  }

  // Sensirion (temperature, humidity)
  if (UIO.action(1, UIO.sensor_sensirion))
  {
    UIO.on(UIO_SENSIRION);
    CR_DELAY(50);
    temperature = SensorAgrv20.readValue(SENS_AGR_SENSIRION, SENSIRION_TEMP);
    humidity = SensorAgrv20.readValue(SENS_AGR_SENSIRION, SENSIRION_HUM);
    ADD_SENSOR(SENSOR_TCB, temperature); // Add digital temperature
    ADD_SENSOR(SENSOR_HUMB, humidity); // Add digital humidity
  }

  // OFF
  if (UIO.action(1, UIO.sensor_leafwetness))
  {
    UIO.off(UIO_LEAFWETNESS);
  }
  if (UIO.action(1, UIO.sensor_sensirion))
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
  if (UIO.action(1, UIO.sensor_ctd10))
  {
    CR_SPAWN2(taskSdiCtd10, tid);
    CR_JOIN(tid);
  }

  // DS-2
  if (UIO.action(1, UIO.sensor_ds2))
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
  uint8_t present, crc, n;
  int16_t temp;
  float temp_f;
  char temp_str[20];
  WaspOneWire oneWire(DIGITAL8); // pin hardcoded

  CR_BEGIN;

  // For now we only support the DS1820, so here just read that directly
  // We assume we have a chain of DS1820 sensors, and read all of them.

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
    // Check address CRC
    crc = oneWire.crc8(addr, 7);
    if (crc != addr[7])
    {
      error(
        F("OneWire %02X%02X%02X%02X%02X%02X%02X%02X bad address, CRC failed: %02X"),
        addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7],
        crc
      );
      break;
    }

    // Check device type
    if (addr[0] == 0x28) // DS18B20
    {
      n++;
    }
    else
    {
      warn(
        F("OneWire %02X%02X%02X%02X%02X%02X%02X%02X unexpected device type: %02X"),
        addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7],
        addr[0]
      );
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
      warn(
        F("OneWire %02X%02X%02X%02X%02X%02X%02X%02X bad data, CRC failed: %02X%02X%02X%02X%02X%02X%02X%02X%02X %02X"),
        addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7],
        data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8],
        crc
      );
    }

    // Convert to float. Formula for the DS18B20 model.
    temp = (data[1] << 8) | data[0];
    temp_f = (float) temp / 16;
    ADD_SENSOR(SENSOR_DS1820, temp_f);

    // Debug
    Utils.float2String(temp_f, temp_str, 2);
    debug(
      F("OneWire %02X%02X%02X%02X%02X%02X%02X%02X : %s"),
      addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7],
      temp_str
    );
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

/**
 * Network
 */

CR_TASK(taskNetwork)
{
  static tid_t tid;

  // Send, once every 3 hours if low battery  and lithium battery
  bool send;
  if (UIO.batteryType == 1)
  {
    send = (
      UIO.hasSD &&
      (UIO.batteryLevel > 75) || (UIO.batteryLevel > 65 && UIO.time.hour % 3 == 0)
    );
  }

  // send period for Lead Acid battery
  else if (UIO.batteryType == 2)
  {
    send = UIO.hasSD;
  }

  CR_BEGIN;

  if (!xbeeDM.XBee_ON)
  {
    if (xbeeDM.ON())
    {
      error(F("startNetwork: xbeeDM.ON() failed"));
      CR_ERROR;
    }
  }
  info(F("Network started"));

  // Spawn first the receive task
  //CR_SPAWN(taskNetworkReceive);

  // Schedule sending frames
  if (send)
  {
    CR_SPAWN2(taskNetworkSend, tid);
  }

  CR_DELAY(8000); // Keep the network open at least for 8s

  if (send)
  {
    CR_JOIN(tid);
  }

  // Stop network
  if (xbeeDM.XBee_ON)
  {
    xbeeDM.OFF();
    info(F("Network stopped"));
  }

  CR_END;
}

CR_TASK(taskNetworkSend)
{
  SdFile archive;
  uint32_t fileSize;
  uint8_t item[8];
  uint32_t t0;
  char dataFilename[18]; // /data/YYMMDD.txt
  int size;

  CR_BEGIN;

  UIO.startSD();

  // Delay sending of frame by a random time within 50 to 550 ms to avoid
  // jaming the network. XXX
  // CR_DELAY(rand() % 500);

  // Security check, the file size must be a multiple of 8. If it is not we
  // consider there has been a write error, and we trunctate the file.
  fileSize = UIO.tmpFile.fileSize();
  if (fileSize % 8 != 0)
  {
    UIO.tmpFile.truncate(fileSize - fileSize % 8);
    warn(F("sendFrames: wrong file size (%s), truncated"), UIO.tmpFilename);
  }

  // Send frames
  while (UIO.tmpFile.fileSize() && (! cr.timeout(UIO.start, UIO.send_timeout * 1000)))
  {
    t0 = millis();

    // Read the frame length
    UIO.tmpFile.seekEnd(-8);
    if (UIO.tmpFile.read(item, 8) != 8)
    {
      error(F("sendFrames (%s): read error"), UIO.tmpFilename);
      CR_ERROR;
    }

    // Read the frame
    UIO.getDataFilename(dataFilename, item[0], item[1], item[2]);
    if (!SD.openFile((char*)dataFilename, &archive, O_RDONLY))
    {
      error(F("sendFrames: fail to open %s"), dataFilename);
      CR_ERROR;
    }
    archive.seekSet(*(uint32_t *)(item + 3));
    size = archive.read(SD.buffer, (size_t) item[7]);
    archive.close();

    if (size < 0 || size != (int) item[7])
    {
      error(F("sendFrames: fail to read frame from disk %s"), dataFilename);
      CR_ERROR;
    }

    // Send the frame
    if (xbeeDM.send((char*)UIO.network.rx_address, (uint8_t*)SD.buffer, size) == 1)
    {
      warn(F("sendFrames: Send failure"));
      CR_ERROR;
    }

    // Truncate (pop)
    if (UIO.tmpFile.truncate(UIO.tmpFile.fileSize() - 8) == false)
    {
      error(F("sendFrames: error in tmpFile.truncate"));
      CR_ERROR;
    }

    debug(F("Frame sent in %lu ms"), cr.millisDiff(t0));

    // Give control back
    CR_DELAY(0);
  }

  CR_END;
}

CR_TASK(taskNetworkReceive)
{
  CR_BEGIN;

  while (xbeeDM.XBee_ON)
  {
    if (xbeeDM.available())
    {
      debug(F("receivePacket: data available"));
      // Data is expected to be available before calling this method, that's
      // why we only timout for 50ms, much less should be enough (to be
      // tested).
      if (xbeeDM.receivePacketTimeout(100))
      {
        warn(F("receivePacket: timeout (we will retry)"));
      }
      else
      {
        // RSSI
        UIO.readRSSI2Frame();

        // Proxy call to appropriate handler
        if (strstr((const char*)xbeeDM._payload, "GPS_sync") != NULL)
        {
          UIO.receiveGPSsyncTime();
        }
        else
        {
          warn(F("receivePacket: unexpected packet"));
          // Show data stored in '_payload' buffer indicated by '_length'
          debug(F("Data: %s"), xbeeDM._payload);
          // Show data stored in '_payload' buffer indicated by '_length'
          debug(F("Length: %d"), xbeeDM._length);
          // Show data stored in '_payload' buffer indicated by '_length'
          debug(F("Source MAC Address: %02X%02X%02X%02X%02X%02X%02X%02X"),
                xbeeDM._srcMAC[0],
                xbeeDM._srcMAC[1],
                xbeeDM._srcMAC[2],
                xbeeDM._srcMAC[3],
                xbeeDM._srcMAC[4],
                xbeeDM._srcMAC[5],
                xbeeDM._srcMAC[6],
                xbeeDM._srcMAC[7]
               );
        }
      }
    }

    // Give control back
    CR_DELAY(0);
  }

  CR_END;
}

/**
 * Set RTC time from GPS
 *
 * TODO Use ephemiris, apparently this is required to improve power.
 */

CR_TASK(taskGps)
{
  uint32_t before, after;
  uint32_t start, time;

  // On
  if (GPS.ON() == 0)
  {
    warn(F("GPS: GPS.ON() failed"));
    return CR_TASK_ERROR;
  }

  debug(F("GPS: Start"));
  start = millis();

  // Ephemerides
  if (UIO.hasSD)
  {
    if (GPS.loadEphems() == 1)
    {
      debug(F("GPS: Ephemerides loaded"));
    }
    else
    {
      warn(F("GPS: Ephemerides loading failed"));
    }
  }

  // XXX We could use GPS.check instead, and give control back with CR_DELAY,
  // problem is when we sleep (cr) the gps is powered off (to verify).
  if (GPS.waitForSignal(150) == false) // 150s = 2m30s
  {
    warn(F("GPS: Timeout"));
    GPS.OFF();
    return CR_TASK_ERROR;
  }

  // Ephemerides
  if (UIO.hasSD)
  {
    if (GPS.saveEphems() == 1)
    {
      debug(F("GPS: Ephemerides saved"));
    }
    else
    {
      warn(F("GPS: Ephemerides saving failed"));
    }
  }
  time = millis() - start;

  // Position
  if (GPS.getPosition() != 1)
  {
    warn(F("GPS: getPosition failed"));
    GPS.OFF();
    return CR_TASK_ERROR;
  }

  // Time
  // XXX could optimize, as part of the work in setTimeFromGPS is already done
  // in getPosition above.
  before = UIO.getEpochTime();
  GPS.setTimeFromGPS(); // Save time to RTC
  UIO.loadTime(); // Set system time
  after = UIO.getEpochTime();
  GPS.OFF();

  // Frames
  uint32_t skew = (after > before) ? (after - before): (before - after);
  info(F("GPS: Success, time updated (time=%lu skew=%lu)"), time, skew);
  debug(F("GPS: Position latitude=%s %c"), GPS.latitude, GPS.NS_indicator);
  debug(F("GPS: Position longitude=%s %c"), GPS.longitude, GPS.EW_indicator);
  debug(F("GPS: Position altitude=%s course=%s speed=%s"), GPS.altitude, GPS.course, GPS.speed);
  //ADD_SENSOR(SENSOR_GPS_STATS, time, skew);
  ADD_SENSOR(SENSOR_GPS, GPS.convert2Degrees(GPS.latitude , GPS.NS_indicator),
                         GPS.convert2Degrees(GPS.longitude, GPS.EW_indicator));
  //ADD_SENSOR(SENSOR_ALTITUDE, GPS.altitude);
  //ADD_SENSOR(SENSOR_SPEED, GPS.speed);
  //ADD_SENSOR(SENSOR_COURSE, GPS.course);

  // Off
  return CR_TASK_STOP;
}

/**
 * This task is only to test the watchdog reset.
 */

CR_TASK(taskSlow)
{
  CR_BEGIN;

  // Wait a little bit so this is executed last
  CR_DELAY(12000);

  warn(F("Start slow task"));
  delay(5 * 60000); // 5 minutes
  warn(F("End slow task"));

  CR_END;
}
