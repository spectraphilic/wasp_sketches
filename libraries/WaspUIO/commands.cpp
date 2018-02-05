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

const char CMD_BATTERY [] PROGMEM = "bat VALUE      - Choose the battery type: 1=lithium 2=lead";
const char CMD_CAT     [] PROGMEM = "cat FILENAME   - Print FILENAME contents to USB";
const char CMD_DISABLE [] PROGMEM = "disable FLAG   - Disables a feature: 0=log_sd 1=log_usb";
const char CMD_ENABLE  [] PROGMEM = "enable FLAG    - Enables a feature: 0=log_sd 1=log_usb";
const char CMD_EXIT    [] PROGMEM = "exit           - Exit the command line interface";
const char CMD_FORMAT  [] PROGMEM = "format         - Format SD card";
const char CMD_HELP    [] PROGMEM = "help           - Prints the list of commands";
const char CMD_LOGLEVEL[] PROGMEM = "loglevel VALUE - Sets the log level: "
                                    "0=off 1=fatal 2=error 3=warn 4=info 5=debug 6=trace";
const char CMD_LS      [] PROGMEM = "ls             - List files in SD card";
const char CMD_NETWORK [] PROGMEM = "network VALUE  - Choose network: "
                                    "0=Finse 1=<unused> 2=Broadcast 3=Pi@UiO 4=Pi@Finse 5=Pi@Spain";
const char CMD_ONEWIRE [] PROGMEM = "onewire pin(s) - Identify OneWire sensors attached to the given pins,"
                                    "saves to onewire.txt";
const char CMD_PRINT   [] PROGMEM = "print          - Print configuration and other information";
const char CMD_RUN     [] PROGMEM = "run VALUE MIN  - Run every 0-255 minutes: "
                                    "0=network 1=sensirion 2=pressure 3=lw 4=ctd10 5=ds2 6=ds1820 7=bme280 8=mb";
const char CMD_SDI12   [] PROGMEM = "sdi            - Identify SDI-12 sensors in addresses 0 and 1";
const char CMD_TIME_GPS[] PROGMEM = "time gps       - Sets time from GPS";
const char CMD_TIME    [] PROGMEM = "time VALUE     - Sets time to the given value, format is yy:mm:dd:hh:mm:ss";

const Command commands[] PROGMEM = {
  {"bat ",      &cmdBattery,  CMD_BATTERY},
  {"cat ",      &cmdCat,      CMD_CAT},
  {"disable ",  &cmdDisable,  CMD_DISABLE},
  {"enable ",   &cmdEnable,   CMD_ENABLE},
  {"exit",      &cmdExit,     CMD_EXIT},
  {"format",    &cmdFormat,   CMD_FORMAT},
  {"help",      &cmdHelp,     CMD_HELP},
  {"loglevel ", &cmdLogLevel, CMD_LOGLEVEL},
  {"ls",        &cmdLs,       CMD_LS},
  {"onewire ",  &cmdOneWire,  CMD_ONEWIRE},
  {"network ",  &cmdNetwork,  CMD_NETWORK},
  {"print",     &cmdPrint,    CMD_PRINT},
  {"run ",      &cmdRun,      CMD_RUN},
  {"sdi",       &cmdSDI12,    CMD_SDI12},
  {"time gps",  &cmdTimeGPS,  CMD_TIME_GPS},
  {"time ",     &cmdTime,     CMD_TIME},
};

const uint8_t nCommands = sizeof commands / sizeof commands[0];


/*
 * Command Line Interpreter (cli)
 */

void WaspUIO::clint()
{
  char buffer[150];
  //char out[150];
  size_t size = sizeof(buffer);
  int8_t status;

  // Turn ON
  RTC.ON();
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
      status = exeCommand(buffer);
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
  RTC.OFF();
}


/*
 * Call command
 */

COMMAND(exeCommand)
{
  Command cmd;
  size_t len;

  for (uint8_t i=0; i < nCommands; i++)
  {
    memcpy_P(&cmd, &commands[i], sizeof cmd);
    len = strlen(cmd.prefix);
    if (strncmp(cmd.prefix, str, len) == 0)
    {
      debug(F("command %s"), str);
      return cmd.function(&(str[len]));
    }
  }

  return cmd_bad_input;
}

/**
 * Choose battery type.
 */
COMMAND(cmdBattery)
{
  int value;

  // Check input
  if (sscanf(str, "%d", &value) != 1) { return cmd_bad_input; }
  if (value != 1 && value != 2) { return cmd_bad_input; }

  // Do
  UIO.batteryType = (uint8_t) value;
  UIO.updateEEPROM(EEPROM_UIO_BATTERY_TYPE, UIO.batteryType);
  UIO.readBattery();
  return cmd_ok;
}

/**
 * Choose battery type.
 */
COMMAND(cmdCat)
{
  char filename[80];

  // Check input
  if (sscanf(str, "%s", filename) != 1) { return cmd_bad_input; }
  if (strlen(filename) == 0) { return cmd_bad_input; }

  // Check feature availability
  if (! UIO.hasSD) { return cmd_unavailable; }

  // Do
  SD.showFile((char*) filename);
  return cmd_quiet;
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
  UIO.updateEEPROM(EEPROM_UIO_FLAGS, UIO.flags);
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
  UIO.updateEEPROM(EEPROM_UIO_FLAGS, UIO.flags);
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
 * Format SD card.
 */

COMMAND(cmdFormat)
{
  // Check feature availability
  if (! UIO.hasSD) { return cmd_unavailable; }

  // Do
  SD.format();
  UIO.stopSD(); UIO.startSD(); // Create base files
  return cmd_ok;
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
    cr.println(help);
  }

  return cmd_quiet;
}

/**
 * Sets log level.
 */

COMMAND(cmdLogLevel)
{
  unsigned int value;

  // Check input
  if (sscanf(str, "%u", &value) != 1) { return cmd_bad_input; }
  if (value >= LOG_LEN) { return cmd_bad_input; }

  // Do
  cr.loglevel = (loglevel_t) value;
  UIO.updateEEPROM(EEPROM_UIO_LOG_LEVEL, (uint8_t) cr.loglevel);
  return cmd_ok;
}

/**
 * List files in SD.
 */

COMMAND(cmdLs)
{
  // Check feature availability
  if (! UIO.hasSD) { return cmd_unavailable; }

  // Do
  SD.ls(LS_DATE | LS_SIZE | LS_R);
  return cmd_quiet;
}

/**
 * Print configuration and other information
 */

COMMAND(cmdNetwork)
{
  unsigned int value;

  // Check input
  if (sscanf(str, "%d", &value) != 1) { return cmd_bad_input; }
  if (value >= NETWORK_LEN) { return cmd_bad_input; }

  // Do
  memcpy_P(&UIO.network, &networks[(network_t) value], sizeof UIO.network);
  if (! UIO.updateEEPROM(EEPROM_UIO_NETWORK, UIO.network.panid[0]) ||
      ! UIO.updateEEPROM(EEPROM_UIO_NETWORK+1, UIO.network.panid[1]))
  {
    error(F("ERROR Saving network id to EEPROM failed"));
    return cmd_error;
  }
  UIO.initNet();

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
  PWR.setSensorPower(SENS_3V3, SENS_ON);
  delay(750);

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
  PWR.setSensorPower(SENS_3V3, SENS_OFF);
  if (has_file)
  {
    file.close();
  }

  return cmd_quiet;
}

/**
 * Print configuration and other information
 */

COMMAND(cmdPrint)
{
  char buffer[150];
  size_t size = sizeof(buffer);

  cr.println(F("Time      : %s"), RTC.getTime());
  cr.println(F("Hardware  : Version=%c Mote=%s XBee=%s"), _boot_version,
             UIO.pprintSerial(buffer, sizeof buffer), UIO.myMac);
  cr.println(F("Autodetect: SD=%d GPS=%d"), UIO.hasSD, UIO.hasGPS);
  cr.println(F("Battery   : %s (%d %%)"), UIO.pprintBattery(buffer, size), UIO.batteryLevel);
  cr.println(F("Log       : level=%s output=%s"), cr.loglevel2str(cr.loglevel), UIO.pprintLog(buffer, size));
  cr.println(F("Network   : %s (frame size is %d)"), UIO.pprintNetwork(buffer, size), frame.getFrameSize());
  cr.println(F("Actions   : %s"), UIO.pprintActions(buffer, size));

  return cmd_quiet;
}

/**
 * Running things at time intervals.
 */

COMMAND(cmdRun)
{
  unsigned int what;
  unsigned int minutes;

  // Check input
  if (sscanf(str, "%u %u", &what, &minutes) != 2) { return cmd_bad_input; }
  if (what >= RUN_LEN) { return cmd_bad_input; }
  if (minutes > 255) { return cmd_bad_input; }

  // Do
  UIO.actions[what] = (uint8_t) minutes;
  UIO.updateEEPROM(EEPROM_RUN + what, UIO.actions[what]);
  return cmd_ok;
}

/**
 * SDI-12
 */

COMMAND(cmdSDI12)
{
  // Do
  PWR.setSensorPower(SENS_5V, SENS_ON);
  mySDI12.begin();
  mySDI12.identification(0);
  mySDI12.identification(1);
  mySDI12.end();
  PWR.setSensorPower(SENS_5V, SENS_OFF);
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

  if (sscanf(str, "%hu:%hu:%hu:%hu:%hu:%hu", &year, &month, &day, &hour, &minute, &second) == 6)
  {
    // time=yy:mm:dd:hh:mm:ss
  }
  else if (sscanf(str, "%lu", &epoch) == 1)
  {
    // time=epoch
    //epoch = epoch + xxx; // TODO Add half the round trup time
    RTC.breakTimeAbsolute(epoch, &time);
    year = time.year;
    month = time.month;
    day = time.date;
    hour = time.hour;
    minute = time.minute;
    second = time.second;
  }
  else
  {
    return cmd_bad_input;
  }

  // Do
  if (UIO.saveTime(year, month, day, hour, minute, second) != 0)
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

COMMAND(cmdTimeGPS)
{
  uint32_t before, after;
  uint32_t start, time;

  // Check feature availability
  if (! UIO.hasGPS) { return cmd_unavailable; }

  // On
  if (GPS.ON() == 0)
  {
    warn(F("GPS: GPS.ON() failed"));
    return cmd_error;
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
    return cmd_error;
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
    return cmd_error;
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
  return cmd_ok;
}
