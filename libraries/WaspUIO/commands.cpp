/*
 * The signature of commands is:
 *
 *   int8_t cmd(const char* command);
 *
 * Return:
 *
 *   0: success
 *   1: error, bad input
 *   2: another error
 *
 */

#include "WaspUIO.h"


typedef struct {
  char prefix[12];
  cmd_status_t (*function)(const char* command);
  const char* help;
} Command;

const char CMD_4G      [] PROGMEM = "4g               - Test 4G data connection";
const char CMD_ACK     [] PROGMEM = ""; // Hidden command
const char CMD_APN     [] PROGMEM = "apn [APN]        - Set 4G Access Point Name (APN)";
const char CMD_BATTERY [] PROGMEM = "bat VALUE        - Choose the battery type: 1=lithium 2=lead";
const char CMD_BOARD   [] PROGMEM = "board VALUE      - Choose the sensor board: 0=none 1=lemming";
const char CMD_CAT     [] PROGMEM = "cat FILENAME     - Print FILENAME contents to USB";
const char CMD_CATX    [] PROGMEM = "catx FILENAME    - Print FILENAME contents in hexadecimal to USB";
const char CMD_DISABLE [] PROGMEM = "disable FLAG     - Disables a feature: 0=log_sd 1=log_usb";
const char CMD_ENABLE  [] PROGMEM = "enable FLAG      - Enables a feature: 0=log_sd 1=log_usb";
const char CMD_EXIT    [] PROGMEM = "exit             - Exit the command line interface";
const char CMD_FORMAT  [] PROGMEM = "format           - Format SD card";
const char CMD_GPS     [] PROGMEM = "gps              - Gets position from GPS";
const char CMD_HELP    [] PROGMEM = "help             - Prints the list of commands";
const char CMD_I2C     [] PROGMEM = "i2c              - Scan I2C slaves";
const char CMD_LOGLEVEL[] PROGMEM = "loglevel VALUE   - Sets the log level: "
                                    "0=off 1=fatal 2=error 3=warn 4=info 5=debug 6=trace";
const char CMD_LS      [] PROGMEM = "ls               - List files in SD card";
const char CMD_NAME    [] PROGMEM = "name             - Give a name to the mote (max 16 chars)";
const char CMD_NETWORK [] PROGMEM = "network VALUE    - Choose network type: 0=xbee 1=4g";
const char CMD_ONEWIRE [] PROGMEM = "onewire pin(s)   - Identify OneWire sensors attached to the given pins,"
                                    "saves to onewire.txt";
const char CMD_PASSWORD[] PROGMEM = "password VALUE   - password for frame encryption";
const char CMD_PIN     [] PROGMEM = "pin VALUE        - set pin for the 4G module (0=disabled)";
const char CMD_PRINT   [] PROGMEM = "print            - Print configuration and other information";
const char CMD_READ    [] PROGMEM = "read NAME        - Read sensor: "
                                    "bat(tery) ds1820 bme(280) mb l-as(726x) l-bme(280) l-mlx(90614) l-vl(53l1x)";
const char CMD_RM      [] PROGMEM = "rm FILENAME      - Remove file";
const char CMD_RUN     [] PROGMEM = "run NAME H M     - Run every hours and minutes: "
                                    "net(work) bat(tery) gps ctd(10) ds2 ds1820 bme(280) mb ws100";
const char CMD_SDI12   [] PROGMEM = "sdi [ADDR] [NEW] - Identify SDI-12 sensors";
const char CMD_TAIL    [] PROGMEM = "tail N FILENAME  - Print last N lines of FILENAME to USB";
const char CMD_TIME    [] PROGMEM = "time VALUE       - Sets time, value can be 'network', 'gps', yy:mm:dd:hh:mm:ss "
                                    "or epoch";
const char CMD_XBEE    [] PROGMEM = "xbee VALUE       - Choose xbee network: "
                                    "0=Finse 1=<unused> 2=Broadcast 3=Pi@UiO 4=Pi@Finse 5=Pi@Spain";

const Command commands[] PROGMEM = {
  {"4g",        &cmd4G,       CMD_4G},
  {"ack",       &cmdAck,      CMD_ACK}, // Internal use only
  {"apn",       &cmdAPN,      CMD_APN},
  {"bat ",      &cmdBattery,  CMD_BATTERY},
  {"board ",    &cmdBoard,    CMD_BOARD},
  {"cat ",      &cmdCat,      CMD_CAT},
  {"catx ",     &cmdCatx,     CMD_CATX},
  {"disable ",  &cmdDisable,  CMD_DISABLE},
  {"enable ",   &cmdEnable,   CMD_ENABLE},
  {"exit",      &cmdExit,     CMD_EXIT},
  {"format",    &cmdFormat,   CMD_FORMAT},
  {"gps",       &cmdGPS,      CMD_GPS},
  {"help",      &cmdHelp,     CMD_HELP},
  {"i2c",       &cmdI2C,      CMD_I2C},
  {"loglevel ", &cmdLogLevel, CMD_LOGLEVEL},
  {"ls",        &cmdLs,       CMD_LS},
  {"name",      &cmdName,     CMD_NAME},
  {"onewire ",  &cmdOneWire,  CMD_ONEWIRE},
  {"network ",  &cmdNetwork,  CMD_NETWORK},
  {"password ", &cmdPassword, CMD_PASSWORD},
  {"pin ",      &cmdPin,      CMD_PIN},
  {"print",     &cmdPrint,    CMD_PRINT},
  {"read ",     &cmdRead,     CMD_READ},
  {"rm ",       &cmdRm,       CMD_RM},
  {"run ",      &cmdRun,      CMD_RUN},
  {"sdi",       &cmdSDI12,    CMD_SDI12},
  {"tail",      &cmdTail,     CMD_TAIL},
  {"time ",     &cmdTime,     CMD_TIME},
  {"xbee ",     &cmdXBee,     CMD_XBEE},
};

const uint8_t nCommands = sizeof commands / sizeof commands[0];

const char RUN_NETWORK_NAME [] PROGMEM = "net";            // 0 network
const char RUN_BATTERY_NAME [] PROGMEM = "bat";            // 1 battery
const char RUN_GPS_NAME     [] PROGMEM = "gps";            // 2 gps
const char RUN_FREE_1_NAME  [] PROGMEM = "";
const char RUN_CTD10_NAME   [] PROGMEM = "ctd";            // 4 water
const char RUN_DS2_NAME     [] PROGMEM = "ds2";            // 5 wind
const char RUN_DS1820_NAME  [] PROGMEM = "ds1820";         // 6 temperature string
const char RUN_BME280_NAME  [] PROGMEM = "bme";            // 7 atmospheric (internal)
const char RUN_MB_NAME      [] PROGMEM = "mb";             // 8 sonar
const char RUN_WS100_NAME   [] PROGMEM = "ws100";          // 9 rain
const char RUN_LAGOPUS_AS726X_NAME   [] PROGMEM = "l-as";  // 10 spectrum
const char RUN_LAGOPUS_BME280_NAME   [] PROGMEM = "l-bme"; // 11 atmospheric
const char RUN_LAGOPUS_MLX90614_NAME [] PROGMEM = "l-mlx"; // 12 infrared thermometer
const char RUN_LAGOPUS_TMP102_NAME   [] PROGMEM = "l-tmp"; // 13 digital temperature
const char RUN_LAGOPUS_VL53L1X_NAME  [] PROGMEM = "l-vl";  // 14 distance

const char* const run_names[] PROGMEM = {
  RUN_NETWORK_NAME,
  RUN_BATTERY_NAME,
  RUN_GPS_NAME,
  RUN_FREE_1_NAME,
  RUN_CTD10_NAME,
  RUN_DS2_NAME,
  RUN_DS1820_NAME,
  RUN_BME280_NAME,
  RUN_MB_NAME,
  RUN_WS100_NAME,
  RUN_LAGOPUS_AS726X_NAME,
  RUN_LAGOPUS_BME280_NAME,
  RUN_LAGOPUS_MLX90614_NAME,
  RUN_LAGOPUS_TMP102_NAME,
  RUN_LAGOPUS_VL53L1X_NAME,
};


/*
 * Command Line Interpreter (cli)
 */

void WaspUIO::clint()
{
  char buffer[150];
  //char out[150];
  size_t size = sizeof(buffer);

  // Turn ON
  UIO.startSD();

  // Print info
  cr.println();
  cmdPrint(NULL);
  cr.println();

  // Go interactive or not
  cr.println(F("Press Enter to start interactive mode. Wait 2 seconds to skip."));
  if (cr.input(buffer, sizeof(buffer), 2000) != NULL)
  {
    cr.println(F("Type 'help' to see the list of commands, 'exit' to leave."));
    do {
      cr.print(F("> "));
      cr.input(buffer, size, 0);
      cr.println(buffer);
      int8_t status = exeCommand(buffer);
      if      (status == cmd_bad_input)   { cr.println(F("I don't understand")); }
      else if (status == cmd_unavailable) { cr.println(F("Feature not available")); }
      else if (status == cmd_error)       { cr.println(F("Error")); }
      else if (status == cmd_ok)          { cr.println(F("OK")); }
      else if (status == cmd_quiet)       { }
      else if (status == cmd_exit)        { cr.println(F("Good bye!")); break; }
    } while (true);
  }
  else
  {
    cr.println(F("Timeout."));
  }

  cr.println();

  // Turn OFF
  UIO.stopSD();
}


/*
 * Call command
 */

COMMAND(exeCommand)
{
  Command cmd;
  cmd_status_t status;

  for (uint8_t i=0; i < nCommands; i++)
  {
    memcpy_P(&cmd, &commands[i], sizeof cmd);
    size_t len = strlen(cmd.prefix);
    if (strncmp(cmd.prefix, str, len) == 0)
    {
      debug(F("command %s"), str);
      UIO.saveState();
      status = cmd.function(&(str[len]));
      UIO.loadState();
      return status;
    }
  }

  return cmd_bad_input;
}


/**
 * Test 4G data connection
 */

COMMAND(cmd4G)
{
#if WITH_4G
  uint8_t err = UIO._4GStart();
  if (err)
  {
    return cmd_error;
  }

  UIO._4GStop();
  return cmd_ok;
#else
  error(F("4G not enabled, define WITH_4G TRUE"));
  return cmd_error;
#endif
}


/**
 * Internal: ack frame
 */

COMMAND(cmdAck)
{
  uint8_t item[4];
  uint32_t offset;
  cmd_status_t status = cmd_ok;

  if (UIO.ack_wait == false)
  {
    warn(F("unexpected ack command"));
    return cmd_quiet;
  }

  // Open files
  UIO.startSD();
  if (UIO.openFile(UIO.qstartFilename, UIO.qstartFile, O_RDWR))
  {
    error(cr.last_error);
    return cmd_error;
  }
  if (UIO.openFile(UIO.queueFilename, UIO.queueFile, O_RDWR))
  {
    UIO.qstartFile.close();
    error(cr.last_error);
    return cmd_error;
  }

  // Read offset
  if (UIO.qstartFile.read(item, 4) != 4)
  {
    error(F("ack (%s): read error"), UIO.qstartFilename);
    status = cmd_error;
    goto exit;
  }
  offset = *(uint32_t *)item;

  // Truncate (pop)
  offset += 8;
  if (offset >= UIO.queueFile.fileSize())
  {
    offset = 0;
    if (UIO.queueFile.truncate(0) == false)
    {
      error(F("ack: error in queueFile.truncate"));
      status = cmd_error;
      goto exit;
    }
  }

  // Update offset
  UIO.qstartFile.seekSet(0);
  if (UIO.write(UIO.qstartFile, (void*)(&offset), 4))
  {
    error(F("sendFrames: error updating offset"));
    status = cmd_error;
    goto exit;
  }

  // Ready for next frame!
  UIO.ack_wait = false;

exit:
  UIO.qstartFile.close();
  UIO.queueFile.close();
  return status;
}

/**
 * Set APN (Access Point Name) for 4G module
 */
COMMAND(cmdAPN)
{
#if WITH_4G
  char apn[30];

  // Check input
  int n = sscanf(str, "%29s", apn);
  if (n == -1) // Suprisingly it returns -1 instead of 0
  {
    _4G.show_APN();
  }
  else if (n == 1)
  {
    UIO.writeEEPROM(EEPROM_UIO_APN, apn, sizeof apn);
    _4G.set_APN(apn);
  }
  else
  {
    return cmd_bad_input;
  }

  return cmd_ok;
#else
  error(F("4G not enabled, define WITH_4G TRUE"));
  return cmd_error;
#endif
}

/**
 * Choose battery type.
 */
COMMAND(cmdBattery)
{
  uint8_t value;

  // Check input
  if (sscanf(str, "%hhu", &value) != 1) { return cmd_bad_input; }
  if (value >= BATTERY_LEN) { return cmd_bad_input; }

  // Do
  if (! UIO.updateEEPROM(EEPROM_UIO_BATTERY_TYPE, value)) { return cmd_error; }
  UIO.batteryType = (battery_type_t) value;
  UIO.readBattery();

  return cmd_ok;
}

/**
 * Choose the sensor board type.
 */
COMMAND(cmdBoard)
{
  uint8_t value;

  // Check input
  if (sscanf(str, "%hhu", &value) != 1) { return cmd_bad_input; }
  if (value >= BOARD_LEN) { return cmd_bad_input; }

  // Do
  if (! UIO.updateEEPROM(EEPROM_UIO_BOARD_TYPE, value)) { return cmd_error; }
  UIO.boardType = (board_type_t) value;
  UIO.readBattery();

  return cmd_ok;
}


/**
 * Enable/Disable features (flags).
 */

uint8_t _getFlag(const char* str)
{
  unsigned int flag;

  if (sscanf(str, "%u", &flag) != 1) { return cmd_bad_input; }
  if (flag == 0) return FLAG_LOG_SD;
  if (flag == 1) return FLAG_LOG_USB;

  return 0; // Not found
}

COMMAND(cmdDisable)
{
  uint8_t flag;

  // Check input
  flag = _getFlag(str);
  if (flag == 0) { return cmd_bad_input; }

  // Do
  UIO.flags &= ~flag;
  if (! UIO.updateEEPROM(EEPROM_UIO_FLAGS, UIO.flags)) { return cmd_error; }

  return cmd_ok;
}

COMMAND(cmdEnable)
{
  uint8_t flag;

  // Check input
  flag = _getFlag(str);
  if (flag == 0) { return cmd_bad_input; }

  // Do
  UIO.flags |= flag;
  if (! UIO.updateEEPROM(EEPROM_UIO_FLAGS, UIO.flags)) { return cmd_error; }

  return cmd_ok;
}

/**
 * Exit the command line interface
 */

COMMAND(cmdExit)
{
  return cmd_exit;
}

/**
 * Help: Prints the list of commands
 */

COMMAND(cmdHelp)
{
  Command cmd;
  char help[120];

  for (uint8_t i=0; i < nCommands; i++)
  {
    memcpy_P(&cmd, &commands[i], sizeof cmd);
    strncpy_P(help, cmd.help, sizeof help);
    if (strlen(help) > 0) // Do not print help for hidden commands
    {
      cr.println(help);
    }
  }

  return cmd_quiet;
}

/**
 * Scan I2C slaves
 */

COMMAND(cmdI2C)
{
  cr.println(F("EEPROM           (%02x) %hhu"), I2C_ADDRESS_EEPROM, I2C.scan(I2C_ADDRESS_EEPROM));
  cr.println(F("RTC              (%02x) %hhu"), I2C_ADDRESS_WASP_RTC, I2C.scan(I2C_ADDRESS_WASP_RTC));
  cr.println(F("ACC              (%02x) %hhu"), I2C_ADDRESS_WASP_ACC, I2C.scan(I2C_ADDRESS_WASP_ACC));
  cr.println(F("BME280           (%02x) %hhu"), I2C_ADDRESS_Lemming_BME280, I2C.scan(I2C_ADDRESS_Lemming_BME280));

  cr.println(F("LAGOPUS AS726X   (%02x) %hhu"), I2C_ADDRESS_LAGOPUS_AS726X, I2C.scan(I2C_ADDRESS_LAGOPUS_AS726X));
  cr.println(F("LAGOPUS BME280   (%02x) %hhu"), I2C_ADDRESS_LAGOPUS_BME280, I2C.scan(I2C_ADDRESS_LAGOPUS_BME280));
  cr.println(F("LAGOPUS MLX90614 (%02x) %hhu"), I2C_ADDRESS_LAGOPUS_MLX90614, I2C.scan(I2C_ADDRESS_LAGOPUS_MLX90614));
  cr.println(F("LAGOPUS TMP102   (%02x) %hhu"), I2C_ADDRESS_LAGOPUS_TMP102, I2C.scan(I2C_ADDRESS_LAGOPUS_TMP102));
  cr.println(F("LAGOPUS VL53L1X  (%02x) %hhu"), I2C_ADDRESS_LAGOPUS_VL53L1X, I2C.scan(I2C_ADDRESS_LAGOPUS_VL53L1X));

  cr.println(F("0=success 1=no-state ..."));
  return cmd_quiet;
}

/**
 * Sets log level.
 */

COMMAND(cmdLogLevel)
{
  uint8_t value;

  // Check input
  if (sscanf(str, "%hhu", &value) != 1) { return cmd_bad_input; }
  if (value >= LOG_LEN) { return cmd_bad_input; }

  // Do
  if (! UIO.updateEEPROM(EEPROM_UIO_LOG_LEVEL, value)) { return cmd_error; }
  cr.loglevel = (loglevel_t) value;

  return cmd_ok;
}


/**
 * Give a name to the mote
 */

COMMAND(cmdName)
{
  char value[17];

  // Check input
  if (sscanf(str, "%16s", value) != 1) { return cmd_bad_input; }
  if (strlen(value) == 0) { return cmd_bad_input; }

  Utils.setID(value);
  return cmd_ok;
}


/**
 * Choose network type
 */

COMMAND(cmdNetwork)
{
  uint8_t value;

  // Check input
  if (sscanf(str, "%hhu", &value) != 1) { return cmd_bad_input; }
  if (value >= NETWORK_LEN) { return cmd_bad_input; }

  // Do
  if (! UIO.updateEEPROM(EEPROM_UIO_NETWORK_TYPE, value)) { return cmd_error; }
  UIO.networkType = (network_type_t) value;

  // Action
  UIO.setFrameSize();
  UIO.networkInit();

  return cmd_ok;
}


/**
 * Autodetect sensors attached to the given OneWire pin
 */

uint8_t _getPin(uint8_t pin)
{
  // 0 - 8
  if (pin == 0) return DIGITAL0;
  if (pin == 1) return DIGITAL1;
  if (pin == 2) return DIGITAL2;
  if (pin == 3) return DIGITAL3;
  if (pin == 4) return DIGITAL4;
  if (pin == 5) return DIGITAL5;
  if (pin == 6) return DIGITAL6;
  if (pin == 7) return DIGITAL7;
  if (pin == 8) return DIGITAL8;

  return 255;
}

COMMAND(cmdOneWire)
{
  uint8_t npins;
  uint8_t pins[] = {255, 255, 255};
  uint8_t pin;
  uint8_t addr[8];
  char addr_str[17];
  uint8_t crc;
  SdFile file;
  bool has_file = false;
  size_t size = 20;
  char buffer[size];

  // Check input
  npins = sscanf(str, "%hhu %hhu %hhu", &pins[0], &pins[1], &pins[2]);
  if (npins < 1) { return cmd_bad_input; }

  // ON
  if (UIO.hasSD)
  {
    if (SD.openFile("onewire.txt", &file, O_WRITE | O_CREAT | O_TRUNC))
    {
      has_file = true;
    }
    else
    {
      cr.print(F("Error opening onewire.txt"));
    }
  }

  if (! UIO.onewire(1)) { delay(750); }

  for (uint8_t i = 0; i < npins; i++)
  {
    pin = _getPin(pins[i]);
    if (pin == 255) continue;

    snprintf_F(buffer, size, F("%hhu"), pins[i]);
    USB.print(buffer); if (has_file) file.write(buffer);
    WaspOneWire oneWire(pin);

    // For now we only support the DS1820, so here just read that directly
    // We assume we have a chain of DS1820 sensors, and read all of them.
    if (! oneWire.reset())
    {
      cr.print(F(" nothing"));
      goto next;
    }

    // Search
    oneWire.reset_search();
    while (oneWire.search(addr))
    {
      Utils.hex2str(addr, addr_str, 8);
      snprintf_F(buffer, size, F(" %s"), addr_str);
      USB.print(buffer); if (has_file) file.write(buffer);

      // Check address CRC
      crc = oneWire.crc8(addr, 7);
      if (crc != addr[7]) { cr.print(F("(crc error)")); continue; }

      // Only DS18B20 is supported for now
      if (addr[0] != 0x28) { cr.print(F("(not DS18B20)")); continue; }
    }

next:
    oneWire.depower();
    USB.println(); if (has_file) file.write("\n");
  }

  // OFF
  UIO.onewire(0);
  if (has_file) { file.close(); }

  return cmd_quiet;
}


/**
 * 4G configuration
 */

COMMAND(cmdPassword)
{
  char value[33];

  // Check input
  if (sscanf(str, "%32s", value) != 1) { return cmd_bad_input; }

  size_t len = strlen(value);
  if (len != 16 && len != 24 && len != 32)
  {
    cr.println(F("Password disabled (valid passwords have 16, 24 or 32 chars)"));
    UIO.password[0] = '\0';
  }
  else
  {
    strcpy(UIO.password, value);
  }

  UIO.writeEEPROM(EEPROM_UIO_PWD, UIO.password, sizeof UIO.password);

  // Frame size depends on whether there's encryption or not
  UIO.setFrameSize();

  return cmd_ok;
}


COMMAND(cmdPin)
{
  unsigned int pin;

  if (sscanf(str, "%u", &pin) != 1) { return cmd_bad_input; }
  if (pin > 9999) { return cmd_bad_input; }

  if (! UIO.updateEEPROM(EEPROM_UIO_PIN, pin)) { return cmd_error; }
  UIO.pin = pin;

  return cmd_ok;
}


/**
 * Print configuration and other information
 */

COMMAND(cmdPrint)
{
  char name[17];
  char buffer[150];
  size_t size = sizeof(buffer);

  Utils.getID(name);

  cr.println(F("Time      : %s"), RTC.getTime());
  cr.println(F("Id        : %s Version=%c Name=%s"), UIO.pprintSerial(buffer, size), _boot_version, name);
  cr.println(F("Battery   : %s"), UIO.pprintBattery(buffer, size));
  cr.println(F("Hardware  : board=%s SD=%d GPS=%d"), UIO.pprintBoard(buffer, size), UIO.hasSD, UIO.hasGPS);

  if (UIO.networkType == NETWORK_XBEE)
  {
    cr.println(F("XBee      : %s"), UIO.pprintXBee(buffer, size));
  }
  else if (UIO.networkType == NETWORK_4G)
  {
    cr.println(F("4G        : %s"), UIO.pprint4G(buffer, size));
  }

  cr.println(F("Frames    : %s"), UIO.pprintFrames(buffer, size));

  cr.println(F("Log       : level=%s output=%s"), cr.loglevel2str(cr.loglevel), UIO.pprintLog(buffer, size));
  cr.println(F("Actions   : %s"), UIO.pprintActions(buffer, size));

  return cmd_quiet;
}

/**
 * Read sensor now
 */

COMMAND(cmdRead)
{
  char name[11];
  int8_t value;
  bool error;

  // Check input
  if (sscanf(str, "%10s", &name) != 1) { return cmd_bad_input; }
  value = UIO.index(run_names, sizeof run_names / sizeof run_names[0], name);

  // Do
  if (value == 1)
  {
    char buffer[25];
    UIO.readBattery();
    cr.println(F("%s"), UIO.pprintBattery(buffer, sizeof buffer));
    return cmd_quiet;
  }
  else if (value == 6)
  {
    uint8_t max = 99;
    int values[max];
    UIO.readDS18B20(values, max);
    error = false;
  }
  else if (value == 7)
  {
    float temperature, humidity, pressure;
    error = UIO.readBME280(temperature, humidity, pressure);
  }
  else if (value == 8)
  {
    uint16_t median, sd;
    error = UIO.readMaxbotixSerial(median, sd, 5);
  }
  else if (value == 10)
  {
    float temp, r, s, t, u, v, w;
    error = UIO.readAS7263(temp, r, s, t, u, v, w);
  }
  else if (value == 11)
  {
    float temperature, humidity, pressure;
    error = UIO.readBME280(temperature, humidity, pressure, I2C_ADDRESS_LAGOPUS_BME280);
  }
  else if (value == 12)
  {
    float object, ambient;
    error = UIO.readMLX90614(object, ambient);
  }
  else if (value == 13)
  {
    float temperature;
    error = UIO.readTMP102(temperature);
  }
  else if (value == 14)
  {
    uint16_t distance;
    error = UIO.readVL53L1X(distance);
  }
  else
  {
    return cmd_bad_input;
  }

  if (error)
  {
    return cmd_error;
  }

  return cmd_ok;
}

/**
 * Running things at time intervals.
 */

COMMAND(cmdRun)
{
  char name[11];
  int8_t value;
  uint8_t hours;
  uint8_t minutes;

  // Check input
  if (sscanf(str, "%10s %hhu %hhu", &name, &hours, &minutes) != 3) { return cmd_bad_input; }
  value = UIO.index(run_names, sizeof run_names / sizeof run_names[0], name);
  if (value == -1) { return cmd_bad_input; }

  if (minutes > 59) { return cmd_bad_input; }

  // Do
  uint16_t base = EEPROM_UIO_RUN + (value * 2);
  if (! UIO.updateEEPROM(base, hours)) { return cmd_error; }
  if (! UIO.updateEEPROM(base + 1, minutes)) { return cmd_error; }
  UIO.actions[value] = (hours * 60) + minutes;

  return cmd_ok;
}

/**
 * SDI-12
 */

COMMAND(cmdSDI12)
{
  uint8_t address, new_address;

  int n = sscanf(str, "%hhu %hhu", &address, &new_address);

  UIO.sdi12(1);
  if (n <= 0)
  {
    sdi12.read_address();
  }
  else if (n == 1)
  {
    sdi12.identify(address);
  }
  else if (n == 2)
  {
    sdi12.set_address(address, new_address);
  }
  UIO.sdi12(0);

  return cmd_quiet;
}


/**
 * Sets time. Accepts two formats:
 *
 * - time=yy:mm:dd:hh:mm:ss
 * - time=<seconds since the epoch>
 *
 */

COMMAND(cmdTime)
{
  unsigned short year, month, day, hour, minute, second;
  unsigned long epoch;
  timestamp_t time;
  uint8_t err;

  if (strcmp(str, "network") == 0)
  {
    return (UIO.setTimeFromNetwork()) ? cmd_error : cmd_ok;
  }

  if (strcmp(str, "gps") == 0)
  {
    return (UIO.gps(true, false) == -1) ? cmd_error : cmd_ok;
  }

  if (sscanf(str, "%hu:%hu:%hu:%hu:%hu:%hu", &year, &month, &day, &hour, &minute, &second) == 6)
  {
    // time yy:mm:dd:hh:mm:ss
    err = UIO.setTime(year, month, day, hour, minute, second);
  }
  else if (sscanf(str, "%lu", &epoch) == 1)
  {
    // time epoch
    // XXX Add half the round trip time to epoch (if command comes from network)
    err = UIO.setTime(epoch);
  }
  else
  {
    return cmd_bad_input;
  }

  // Error
  if (err != 0)
  {
    error(F("Failed to save time %lu"), epoch);
    return cmd_error;
  }

  return cmd_ok;
}

/**
 * Sets time. Accepts two formats:
 *
 * - time=yy:mm:dd:hh:mm:ss
 * - time=<seconds since the epoch>
 *
 */

COMMAND(cmdGPS)
{
  // Check feature availability
  if (! UIO.hasGPS) { return cmd_unavailable; }

  if (UIO.gps(false, true) == -1)
  {
    return cmd_error;
  }

  return cmd_ok;
}


/**
 * Choose Xbee network
 */

COMMAND(cmdXBee)
{
  uint8_t value;

  // Check input
  if (sscanf(str, "%hhu", &value) != 1) { return cmd_bad_input; }
  if (value >= xbee_len) { return cmd_bad_input; }

  // Do
  memcpy_P(&UIO.xbee, &xbees[value], sizeof UIO.xbee);
  if (! UIO.updateEEPROM(EEPROM_UIO_XBEE, UIO.xbee.panid[0]) ||
      ! UIO.updateEEPROM(EEPROM_UIO_XBEE+1, UIO.xbee.panid[1]))
  {
    return cmd_error;
  }
  UIO.xbeeInit();

  return cmd_ok;
}
