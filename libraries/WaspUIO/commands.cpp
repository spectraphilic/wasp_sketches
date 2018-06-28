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

const char CMD_ACK     [] PROGMEM = ""; // Hidden command
const char CMD_BATTERY [] PROGMEM = "bat VALUE       - Choose the battery type: 1=lithium 2=lead";
const char CMD_BOARD   [] PROGMEM = "board VALUE     - Choose the sensor board: 0=none 1=lemming";
const char CMD_CAT     [] PROGMEM = "cat FILENAME    - Print FILENAME contents to USB";
const char CMD_CATX    [] PROGMEM = "catx FILENAME   - Print FILENAME contents in hexadecimal to USB";
const char CMD_DISABLE [] PROGMEM = "disable FLAG    - Disables a feature: 0=log_sd 1=log_usb";
const char CMD_ENABLE  [] PROGMEM = "enable FLAG     - Enables a feature: 0=log_sd 1=log_usb";
const char CMD_EXIT    [] PROGMEM = "exit            - Exit the command line interface";
const char CMD_FORMAT  [] PROGMEM = "format          - Format SD card";
const char CMD_HELP    [] PROGMEM = "help            - Prints the list of commands";
const char CMD_LOGLEVEL[] PROGMEM = "loglevel VALUE  - Sets the log level: "
                                    "0=off 1=fatal 2=error 3=warn 4=info 5=debug 6=trace";
const char CMD_LS      [] PROGMEM = "ls              - List files in SD card";
const char CMD_NAME    [] PROGMEM = "name            - Give a name to the mote (max 16 chars)";
const char CMD_NETWORK [] PROGMEM = "network VALUE   - Choose network type: 0=xbee 1=4g";
const char CMD_ONEWIRE [] PROGMEM = "onewire pin(s)  - Identify OneWire sensors attached to the given pins,"
                                    "saves to onewire.txt";
const char CMD_PIN     [] PROGMEM = "pin VALUE       - set pin for the 4G module";
const char CMD_PRINT   [] PROGMEM = "print           - Print configuration and other information";
const char CMD_READ    [] PROGMEM = "read VALUE      - Read sensor: 1=battery 6=ds1820 8=mb";
const char CMD_RUN     [] PROGMEM = "run VALUE MIN   - Run every 0-255 minutes: 0=network 1=battery "
                                    "4=ctd10 5=ds2 6=ds1820 7=bme280 8=mb";
const char CMD_SDI12   [] PROGMEM = "sdi             - Identify SDI-12 sensors in addresses 0 and 1";
const char CMD_TAIL    [] PROGMEM = "tail N FILENAME - Print last N lines of FILENAME to USB";
const char CMD_TIME_GPS[] PROGMEM = "time gps        - Sets time from GPS";
const char CMD_TIME    [] PROGMEM = "time VALUE      - Sets time to the given value, format is yy:mm:dd:hh:mm:ss";
const char CMD_XBEE    [] PROGMEM = "xbee VALUE      - Choose xbee network: "
                                    "0=Finse 1=<unused> 2=Broadcast 3=Pi@UiO 4=Pi@Finse 5=Pi@Spain";

const Command commands[] PROGMEM = {
  {"ack",       &cmdAck,      CMD_ACK}, // Internal use only
  {"bat ",      &cmdBattery,  CMD_BATTERY},
  {"board ",    &cmdBoard,    CMD_BOARD},
  {"cat ",      &cmdCat,      CMD_CAT},
  {"catx ",     &cmdCatx,     CMD_CATX},
  {"disable ",  &cmdDisable,  CMD_DISABLE},
  {"enable ",   &cmdEnable,   CMD_ENABLE},
  {"exit",      &cmdExit,     CMD_EXIT},
  {"format",    &cmdFormat,   CMD_FORMAT},
  {"help",      &cmdHelp,     CMD_HELP},
  {"loglevel ", &cmdLogLevel, CMD_LOGLEVEL},
  {"ls",        &cmdLs,       CMD_LS},
  {"name",      &cmdName,     CMD_NAME},
  {"onewire ",  &cmdOneWire,  CMD_ONEWIRE},
  {"network ",  &cmdNetwork,  CMD_NETWORK},
  {"pin ",      &cmdPin,      CMD_PIN},
  {"print",     &cmdPrint,    CMD_PRINT},
  {"read ",     &cmdRead,     CMD_READ},
  {"run ",      &cmdRun,      CMD_RUN},
  {"sdi",       &cmdSDI12,    CMD_SDI12},
  {"tail",      &cmdTail,     CMD_TAIL},
  {"time gps",  &cmdTimeGPS,  CMD_TIME_GPS},
  {"time ",     &cmdTime,     CMD_TIME},
  {"xbee ",     &cmdXBee,     CMD_XBEE},
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
  cmd_status_t status;

  for (uint8_t i=0; i < nCommands; i++)
  {
    memcpy_P(&cmd, &commands[i], sizeof cmd);
    len = strlen(cmd.prefix);
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
 *  Print to USB FILENAME.
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

COMMAND(cmdCatx)
{
  char filename[80];
  SdFile file;
  uint32_t size;
  uint8_t idx;
  int chr;

  // Check input
  if (sscanf(str, "%s", filename) != 1) { return cmd_bad_input; }
  if (strlen(filename) == 0) { return cmd_bad_input; }

  // Check feature availability
  if (! UIO.hasSD) { return cmd_unavailable; }

  // Open file
  if (! SD.openFile(filename, &file, O_READ)) { return cmd_error; }

  size = file.fileSize();
  for (idx=0; idx < size; idx++)
  {
    chr = file.read();
    if (chr < 0)
    {
      file.close();
      return cmd_error;
    }
    USB.printHex((char)chr);
    USB.print(" ");
  }

  file.close();
  USB.println();

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
    if (strlen(help) > 0) // Do not print help for hidden commands
    {
      cr.println(help);
    }
  }

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
 * Give a name to the mote
 */

COMMAND(cmdName)
{
  char value[17];

  // Check input
  if (sscanf(str, "%16s", &value) != 1) { return cmd_bad_input; }
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
  // XXX Action?

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

COMMAND(cmdPin)
{
  uint16_t pin;

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
    cr.println(F("4G        : pin=XXXX"));
  }

  cr.println(F("Log       : level=%s output=%s"), cr.loglevel2str(cr.loglevel), UIO.pprintLog(buffer, size));
  cr.println(F("Actions   : %s"), UIO.pprintActions(buffer, size));

  return cmd_quiet;
}

/**
 * Read sensor now
 */

COMMAND(cmdRead)
{
  unsigned int value;

  // Check input
  if (sscanf(str, "%u", &value) != 1) { return cmd_bad_input; }
  if (value != 1 && value != 6 && value != 8) { return cmd_bad_input; }

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
    uint8_t max = 40;
    int values[max];
    UIO.readDS18B20(values, max);
  }
  else if (value == 8)
  {
    uint16_t median, sd;
    UIO.readMaxbotixSerial(median, sd, 5);
  }

  return cmd_ok;
}

/**
 * Running things at time intervals.
 */

COMMAND(cmdRun)
{
  uint8_t what;
  uint8_t minutes;

  // Check input
  if (sscanf(str, "%hhu %hhu", &what, &minutes) != 2) { return cmd_bad_input; }
  if (what >= RUN_LEN) { return cmd_bad_input; }
  if (minutes > 255) { return cmd_bad_input; }

  // Do
  if (! UIO.updateEEPROM(EEPROM_UIO_RUN + what, minutes)) { return cmd_error; }
  UIO.actions[what] = minutes;

  return cmd_ok;
}

/**
 * SDI-12
 */

COMMAND(cmdSDI12)
{
  UIO.sdi12(1);
  mySDI12.begin();
  mySDI12.identification(0);
  mySDI12.identification(1);
  mySDI12.end();
  UIO.sdi12(0);
  return cmd_quiet;
}

/**
 * Print Tail lines of FILENAME to USB
 */

COMMAND(cmdTail)
{
  unsigned int maxnl, nl;
  char filename[80];
  SdFile file;
  uint32_t size, nc;
  uint16_t max = DOS_BUFFER_SIZE - 1;
  int16_t c;
  size_t nbyte;
  int nread;

  // Check input
  if (sscanf(str, "%u %s", &maxnl, &filename) != 2) { return cmd_bad_input; }
  if (strlen(filename) == 0) { return cmd_bad_input; }

  // Check feature availability
  if (! UIO.hasSD) { return cmd_unavailable; }

  // Open file
  if (!SD.openFile(filename, &file, O_READ)) { return cmd_error; }

  // Read backwards
  maxnl++;
  size = file.fileSize();
  nc = 0;
  nl = 0;
  while (nc < size && nl < maxnl)
  {
    nc++;
    file.seekEnd(-nc);
    if ((c = file.read()) < 0) { goto error; }
    if (c == '\n') { nl++; }
  }
  nc--; // do not include the last newline read

  // Read forward
  cr.println(F("-------------------------"));
  file.seekEnd(-nc);
  while (nc > 0)
  {
    nbyte = (nc < max) ? nc : max;
    nread = file.read(SD.buffer, nbyte);
    if (nread < 0) { goto error; }
    USB.print(SD.buffer);
    nc -= nread;
  }
  cr.println(F("-------------------------"));

  file.close();
  return cmd_quiet;

error:
  file.close();
  return cmd_error;
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

  // debug(F("GPS: Start"));
  // start = millis();

  // // Ephemerides
  // if (UIO.hasSD)
  // {
  //   if (GPS.loadEphems() == 1)
  //   {
  //     debug(F("GPS: Ephemerides loaded"));
  //   }
  //   else
  //   {
  //     warn(F("GPS: Ephemerides loading failed"));
  //   }
  // }

  // XXX We could use GPS.check instead, and give control back with CR_DELAY,
  // problem is when we sleep (cr) the gps is powered off (to verify).
  if (GPS.waitForSignal(150) == false) // 150s = 2m30s
  {
    warn(F("GPS: Timeout"));
    GPS.OFF();
    return cmd_error;
  }
  else{
    GPS.setTimeFromGPS();
    cr.println(F("Time from GPS updated"));
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
