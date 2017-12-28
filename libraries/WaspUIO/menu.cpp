#include "WaspUIO.h"


void WaspUIO::menu()
{
  char c;
  char buffer[150];
  size_t size = sizeof(buffer);

  RTC.ON();

  // Go interactive or not
  if (input(buffer, sizeof(buffer), F("Press Enter to start interactive mode. Wait 2 seconds to skip:"), 2000) == NULL)
  {
    goto exit;
  }

  do {
    // Menu
    cr.print();
    cr.print(F("1. Time    : %s"), RTC.getTime());
    cr.print(F("2. Log     : level=%s output=%s"), cr.loglevel2str(cr.loglevel), menuFormatLog(buffer, size));
    cr.print(F("3. Programs: sensors (%d min) network (%d min)"), wakeup_sensors, wakeup_network);
    cr.print(F("4. Network : %s"), menuFormatNetwork(buffer, size));
    cr.print(F("5. Sensors : %s"), menuFormatSensors(buffer, size));
    cr.print(F("6. Battery : %s (%d %%)"), menuFormatBattery(buffer, size), batteryLevel);
    if (hasSD)
    {
      cr.print(F("8. SD"));
    }
    else
    {
      cr.print(F("SD card is missing!"));
    }

    cr.print(F("9. Exit"));
    cr.print();
    input(buffer, size, F("==> Enter numeric option:"), 0);
    c = buffer[0];
    if      (c == '1') { menuTime(); }
    else if (c == '2') { menuLog(); }
    else if (c == '3') { menuPrograms(); }
    else if (c == '4') { menuNetwork(); }
    else if (c == '5') { menuSensors(); }
    else if (c == '6') { menuBatteryType(); }
    else if (c == '8') { if (hasSD) menuSD(); }
    else if (c == '9') { goto exit; }
  } while (true);

exit:
  cr.print();
  RTC.OFF();
}

const char* WaspUIO::menuFormatBattery(char* dst, size_t size)
{
  dst[0] = '\0';
  if      (batteryType == 1) strncpy_F(dst, F("Lithium-ion"), size);
  else if (batteryType == 2) strncpy_F(dst, F("Lead acid"), size);
  return dst;
}

const char* WaspUIO::menuFormatLog(char* dst, size_t size)
{
  dst[0] = '\0';
  if (flags & FLAG_LOG_USB) strnjoin_F(dst, size, F(", "), F("USB"));
  if (flags & FLAG_LOG_SD)  strnjoin_F(dst, size, F(", "), F("SD"));
  return dst;
}

const char* WaspUIO::menuFormatNetwork(char* dst, size_t size)
{
  if (flags & FLAG_NETWORK) strncpy(dst, network.name, size);
  else                      strncpy_F(dst, F("Disabled"), size);
  return dst;
}

const char* WaspUIO::menuFormatSensors(char* dst, size_t size)
{
  dst[0] = '\0';
#ifdef USE_AGR
  if (sensor_sensirion)   strnjoin_F(dst, size, F(", "), F("Sensirion (%d)"), sensor_sensirion);
  if (sensor_pressure)    strnjoin_F(dst, size, F(", "), F("Pressure (%d)"), sensor_pressure);
  if (sensor_leafwetness) strnjoin_F(dst, size, F(", "), F("Leaf Wetness (%d)"), sensor_leafwetness);
#endif
#ifdef USE_SDI
  if (sensor_ctd10)       strnjoin_F(dst, size, F(", "), F("CTD-10 (%d)"), sensor_ctd10);
  if (sensor_ds2)         strnjoin_F(dst, size, F(", "), F("DS-2 (%d)"), sensor_ds2);
#endif
#ifdef USE_I2C
  if (sensor_ds1820)      strnjoin_F(dst, size, F(", "), F("DS1820 (%d)"), sensor_ds1820);
  if (sensor_bme280)      strnjoin_F(dst, size, F(", "), F("BME-280 (%d)"), sensor_bme280);
  if (sensor_mb)          strnjoin_F(dst, size, F(", "), F("MB7389 (%d)"), sensor_mb);
#endif
  if (! dst[0])           strncpy_F(dst, F("(none)"), size);
  return dst;
}


/*
 * Menu: time
 */

void WaspUIO::menuTime()
{
  char str[80];

  do
  {
    cr.print();
    cr.print(F("1. Set time manually"));
    if (hasGPS)
    {
      cr.print(F("2. Set time from GPS"));
    }
    cr.print(F("9. Exit"));
    cr.print();
    input(str, sizeof(str), F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '1':
        menuTimeManual();
        break;
      case '2':
        if (hasGPS)
        {
          cr.print(F("Setting time from GPS, please wait, it may take a few minutes"));
          taskGps();
        }
        break;
      case '9':
        cr.print();
        return;
    }
  } while (true);
}


void WaspUIO::menuTimeManual()
{
  unsigned short year, month, day, hour, minute, second;
  char str[80];

  do
  {
    input(str, sizeof(str), F("Set UTC time (format is yy:mm:dd:hh:mm:ss). Press Enter to leave it unchanged:"), 0);
    if (strlen(str) == 0)
      return;

    // Set new time
    if (sscanf(str, "%hu:%hu:%hu:%hu:%hu:%hu", &year, &month, &day, &hour, &minute, &second) == 6)
    {
      saveTime(year, month, day, hour, minute, second);
      cr.print(F("Current time is %s"), RTC.getTime());
      return;
    }
  } while (true);
}


/*
 * Menu: log
 */

const char* WaspUIO::flagStatus(uint8_t flag)
{
  return (flags & flag)? "enabled": "disabled";
}

void WaspUIO::menuLog()
{
  char str[80];
  char* level;

  do
  {
    cr.print();
    cr.print(F("1. Log to SD (%s)"), flagStatus(FLAG_LOG_SD));
    cr.print(F("2. Log to USB (%s)"), flagStatus(FLAG_LOG_USB));
    cr.print(F("3. Choose the log level (%s)"), cr.loglevel2str(cr.loglevel));
    cr.print(F("9. Exit"));
    cr.print();
    input(str, sizeof(str), F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '1':
        menuLog2(FLAG_LOG_SD, "SD");
        break;
      case '2':
        menuLog2(FLAG_LOG_USB, "USB");
        break;
      case '3':
        menuLogLevel();
        break;
      case '9':
        cr.print();
        return;
    }
  } while (true);
}


void WaspUIO::menuLog2(uint8_t flag, const char* var)
{
  char str[80];

  do
  {
    cr.print(F("Type 1 to enable %s output, 0 to disable, Enter to leave:"), var);
    input(str, sizeof(str), F(""), 0);
    if (strlen(str) == 0)
      return;

    switch (str[0])
    {
      case '0':
        UIO.flags &= ~flag;
        updateEEPROM(EEPROM_UIO_FLAGS, UIO.flags);
        return;
      case '1':
        UIO.flags |= flag;
        updateEEPROM(EEPROM_UIO_FLAGS, UIO.flags);
        return;
    }
  } while (true);
}


void WaspUIO::menuLogLevel()
{
  char str[80];

  do
  {
    cr.print();
    cr.print(F("0. Off"));
    cr.print(F("1. Fatal"));
    cr.print(F("2. Error"));
    cr.print(F("3. Warning"));
    cr.print(F("4. Info"));
    cr.print(F("5. Debug"));
    cr.print(F("6. Trace"));
    cr.print(F("9. Exit"));
    cr.print();
    input(str, sizeof(str), F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '0':
        cr.loglevel = LOG_OFF;
        break;
      case '1':
        cr.loglevel = LOG_FATAL;
        break;
      case '2':
        cr.loglevel = LOG_ERROR;
        break;
      case '3':
        cr.loglevel = LOG_WARN;
        break;
      case '4':
        cr.loglevel = LOG_INFO;
        break;
      case '5':
        cr.loglevel = LOG_DEBUG;
        break;
      case '6':
        cr.loglevel = LOG_TRACE;
        break;
      case '9':
        break;
    }
    updateEEPROM(EEPROM_UIO_LOG_LEVEL, (uint8_t) cr.loglevel);
    cr.print();
    return;
  } while (true);
}

/*
 * Menu: network
 */

void WaspUIO::menuNetwork()
{
  char str[80];

  do
  {
    cr.print();
    cr.print(F("0. Disable"));
    cr.print(F("1. Finse"));
    cr.print(F("2. Gateway"));
    cr.print(F("3. Broadcast"));
    cr.print(F("4. Finse alt"));
    cr.print(F("5. Pi Finse"));
    cr.print(F("6. Pi CS (Spain)"));
    cr.print(F("9. Exit"));
    cr.print();
    input(str, sizeof(str), F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '0': // Disable
        UIO.flags &= ~FLAG_NETWORK;
        updateEEPROM(EEPROM_UIO_FLAGS, UIO.flags);
        return;
      case '1':
        setNetwork(NETWORK_FINSE);
        return;
      case '2':
        setNetwork(NETWORK_GATEWAY);
        return;
      case '3':
        setNetwork(NETWORK_BROADCAST);
        return;
      case '4':
        setNetwork(NETWORK_FINSE_ALT);
        return;
      case '5':
        setNetwork(NETWORK_PI_FINSE);
        return;
      case '6':
        setNetwork(NETWORK_PI_CS);
        return;
      case '9':
        cr.print();
        return;
    }
  } while (true);
}

void WaspUIO::setNetwork(network_t value)
{
  // Check input parameter is valid
  if (value < NETWORK_FINSE || NETWORK_PI_CS < value)
  {
      cr.print(F("ERROR No network configuration"));
      return;
  }

  // Enable network
  UIO.flags |= FLAG_NETWORK;
  updateEEPROM(EEPROM_UIO_FLAGS, UIO.flags);

  // Save panID to EEPROM
  memcpy_P(&network, &networks[value], sizeof network);
  if (
    updateEEPROM(EEPROM_UIO_NETWORK, network.panid[0]) &&
    updateEEPROM(EEPROM_UIO_NETWORK+1, network.panid[1])
  )
  {
    cr.print(F("INFO Network id saved to EEPROM"));
  }
  else
  {
    cr.print(F("ERROR Saving network id to EEPROM failed"));
  }

  initNet();
}


void WaspUIO::initNet()
{
  uint8_t addressing = UNICAST_64B;
  network_t value = (network_t) network.panid[1]; // panid low byte
  const __FlashStringHelper * error = NULL;

  // Addressing
  memcpy_P(&network, &networks[value], sizeof network);
  cr.print(F("Configuring network: %s"), network.name);
  if (strcmp(network.rx_address, "000000000000FFFF") == 0)
  {
    addressing = BROADCAST_MODE;
  }

  // This is common to all networks, for now
  frame.setID((char*) "");
  //Utils.setAuthKey(key_access);

  // Set Frame size. Will be 73 bytes for XBeeDM-pro S1
  // linkEncryption = DISABLED (not supported by DIGIMESH, apparently)
  // AESEncryption = DISABLED
  uint16_t size = frame.getMaxSizeForXBee(DIGIMESH, addressing, DISABLED, DISABLED);
  frame.setFrameSize(size);
  cr.print(F("Frame size is %d"), frame.getFrameSize());

  // init XBee
  xbeeDM.ON();
  delay(50);

  readOwnMAC();

  // XXX Reduce the number of retries to reduce the time it is lost in send
  // failures (default is 3).
  // 3 retries ~ 5s ; 2 retries ~ 3.5 s ; 1 retry ~ 2.4s
  xbeeDM.setSendingRetries(2);

  // Set channel, check AT commmand execution flag
  xbeeDM.setChannel(network.channel);
  if (xbeeDM.error_AT)
  {
    error = F("ERROR in setChannel %d");
    goto exit;
  }

  // set PANID, check AT commmand execution flag
  xbeeDM.setPAN(network.panid);
  if (xbeeDM.error_AT)
  {
    error = F("ERROR in setPAN %d");
    goto exit;
  }

  // set encryption mode (1:enable; 0:disable), check AT commmand execution flag
  // XXX Should we use encryption
  xbeeDM.setEncryptionMode(0);
  if (xbeeDM.error_AT)
  {
    error = F("ERROR in setPAN %d");
    goto exit;
  }

  // set encryption key, check AT commmand execution flag
  xbeeDM.setLinkKey(encryptionKey);
  if (xbeeDM.error_AT)
  {
    error = F("ERROR in setLinkKey %d");
    goto exit;
  }

  // write values to XBee module memory, check AT commmand execution flag
  xbeeDM.writeValues();
  if (xbeeDM.error_AT)
  {
    error = F("ERROR in writeValues %d");
    goto exit;
  }

exit:
  xbeeDM.OFF();
  if (error)
  {
    cr.print(error, xbeeDM.error_AT);
  }
}


/*
 * Menu: sensors
 */

void WaspUIO::menuSensors()
{
  char str[80];

  do
  {
    cr.print();
#if USE_AGR
    cr.print(F("1. Agr board: Sensirion (%s)"), sensorStatus(sensor_sensirion));
    cr.print(F("2. Agr board: Pressure (%s)"), sensorStatus(sensor_pressure));
    cr.print(F("3. Agr board: Leaf wetness (%s)"), sensorStatus(sensor_leafwetness));
#endif
#if USE_I2C
    cr.print(F("4. I2C: BME-280 (%s)"), sensorStatus(sensor_bme280));
    cr.print(F("5. OneWire: DS1820 (%s)"), sensorStatus(sensor_ds1820));
    cr.print(F("6. TTL: MB7389 (%s)"), sensorStatus(sensor_mb));
#endif
#if USE_SDI
    cr.print(F("7. SDI-12: CTD-10 (%s)"), sensorStatus(sensor_ctd10));
    cr.print(F("8. SDI-12: DS-2 (%s)"), sensorStatus(sensor_ds2));
#endif
    cr.print(F("9. Exit"));
    cr.print();
    input(str, sizeof(str), F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
#if USE_AGR
      case '1':
        menuSensor(EEPROM_SENSOR_SENSIRION, sensor_sensirion); break;
      case '2':
        menuSensor(EEPROM_SENSOR_PRESSURE, sensor_pressure); break;
      case '3':
        menuSensor(EEPROM_SENSOR_LEAFWETNESS, sensor_leafwetness); break;
#endif
#if USE_I2C
      case '4':
        menuSensor(EEPROM_SENSOR_BME280, sensor_bme280); break;
      case '5':
        menuSensor(EEPROM_SENSOR_DS1820, sensor_ds1820); break;
      case '6':
        menuSensor(EEPROM_SENSOR_MB, sensor_mb); break;
#endif
#if USE_SDI
      case '7':
        menuSensor(EEPROM_SENSOR_CTD10, sensor_ctd10); break;
      case '8':
        menuSensor(EEPROM_SENSOR_DS2, sensor_ds2); break;
#endif
      case '9':
        cr.print();
        return;
    }
  } while (true);
}


void WaspUIO::menuSensor(uint16_t sensor, uint8_t &value)
{
  char str[80];

  do
  {
    cr.print();
    cr.print(F("Current state is: %s"), sensorStatus(value));
    cr.print(F("0. Disable"));
    cr.print(F("1. One period (eg 5 minutes)"));
    cr.print(F("2. Two periods (eg 10 minutes)"));
    cr.print(F("3. Three periods (eg 15 minutes)"));
    if (sensor == EEPROM_SENSOR_CTD10 || sensor == EEPROM_SENSOR_DS2)
    {
      cr.print(F("8. Identification"));
    }
    cr.print(F("9. Exit"));
    cr.print();
    input(str, sizeof(str), F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '0':
        value = 0;
        goto update;
      case '1':
        value = 1;
        goto update;
      case '2':
        value = 2;
        goto update;
      case '3':
        value = 3;
        goto update;
      case 8:
        if (sensor == EEPROM_SENSOR_CTD10 || sensor == EEPROM_SENSOR_DS2)
        {
          cr.print(F("Enabling SDI-12"));
          PWR.setSensorPower(SENS_5V, SENS_ON);
          mySDI12.begin();

          if (sensor == EEPROM_SENSOR_CTD10)
          {
            mySDI12.identification(0);
          }
          else if (sensor == EEPROM_SENSOR_DS2)
          {
            mySDI12.identification(1);
          }

          cr.print(F("Disabling SDI-12"));
          mySDI12.end();
          PWR.setSensorPower(SENS_5V, SENS_OFF);
        }
        break;
      case '9':
        cr.print();
        return;
    }
  } while (true);

update:
  updateEEPROM(sensor, value);
  cr.print();
  return;
}


const char* WaspUIO::sensorStatus(uint8_t sensor)
{
  switch (sensor)
  {
    case 0:
      return "disabled";
    case 1:
      return "1";
    case 2:
      return "2";
    case 3:
      return "3";
    default:
      return "undefined";
  }
}


/*
 * Menu: battery
 */

void WaspUIO::menuBatteryType()
{
  char str[80];
  do{
    cr.print();
    cr.print(F("1. Lithium-ion"));
    cr.print(F("2. Lead acid"));
    cr.print(F("9. Exit"));
    cr.print();

    input(str, sizeof(str), F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '1':
        batteryType = 1;
        break;
      case '2':
        batteryType = 2;
        break;
      case '9':
        return;
      default:
        continue;
    }
    updateEEPROM(EEPROM_UIO_BATTERY_TYPE, batteryType);
    return;
  } while (true);
}


/*
 * Menu: SD
 */

void WaspUIO::menuSD()
{
  char str[80];

  SD.ON();

  do
  {
    cr.print();
    cr.print(F("1. List files"));
    cr.print(F("2. Show file"));
    cr.print(F("3. Format"));
    cr.print(F("9. Exit"));
    cr.print();
    input(str, sizeof(str), F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '1':
        SD.ls(LS_DATE | LS_SIZE | LS_R);
        break;
      case '2':
        input(str, sizeof(str), F("==> Enter path:"), 0);
        if (strlen(str) > 0)
        {
          SD.showFile((char*) str);
        }
        break;
      case '3':
        SD.format();
        break;
      case '9':
        SD.OFF();
        return;
    }
  } while (true);
}
