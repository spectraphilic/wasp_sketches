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
  int8_t (*function)(const char* command);
  const char* help;
} Command;

const char CMD_BATTERY  [] PROGMEM = "bat VALUE      - Choose the battery type: 1=lithium 2=lead";
const char CMD_CAT      [] PROGMEM = "cat FILENAME   - Print FILENAME contents to USB";
const char CMD_DISABLE  [] PROGMEM = "disable FLAG   - Disables a feature";
const char CMD_ENABLE   [] PROGMEM = "enable FLAG    - Enables a feature";
const char CMD_EXIT     [] PROGMEM = "exit           - Exit the command line interface";
const char CMD_FORMAT   [] PROGMEM = "format         - Format SD card";
const char CMD_HELP     [] PROGMEM = "help           - Prints the list of commands";
const char CMD_LOG_LEVEL[] PROGMEM = "loglevel VALUE - Sets the log level: 0=off 1=fatal 2=error 3=warn 4=info 5=debug 6=trace";
const char CMD_LS       [] PROGMEM = "ls             - List files in SD card";
const char CMD_NETWORK  [] PROGMEM = "network VALUE  - Choose network: 0=Finse 1=Gateway 2=Broadcast 3=Finse-alt 4=Pi@Finse 5=Pi@Spain";
const char CMD_PRINT    [] PROGMEM = "print          - Print configuration and other information";
const char CMD_RUN      [] PROGMEM = "run WHAT MIN   - Run every 0-255 minutes: 0=network 1=sensirion 2=pressure 3=lw 4=ctd10 5=ds2 6=ds1820 7=bme280 8=mb";
const char CMD_SDI12    [] PROGMEM = "sdi            - Identify sensors in addresses 0 and 1";
const char CMD_TIME_GPS [] PROGMEM = "time gps       - Sets time from GPS";
const char CMD_TIME     [] PROGMEM = "time VALUE     - Sets time to the given value, format is yy:mm:dd:hh:mm:ss";

const Command commands[] PROGMEM = {
  {"bat",       &cmdBattery,  CMD_BATTERY},
  {"cat ",      &cmdCat,      CMD_CAT},
  {"disable ",  &cmdDisable,  CMD_DISABLE},
  {"enable ",   &cmdEnable,   CMD_ENABLE},
  {"exit",      &cmdExit,     CMD_EXIT},
  {"format ",   &cmdFormat,   CMD_FORMAT},
  {"help",      &cmdHelp,     CMD_HELP},
  {"loglevel ", &cmdLogLevel, CMD_LOG_LEVEL},
  {"ls ",       &cmdLs,       CMD_LS},
  {"network ",  &cmdNetwork,  CMD_NETWORK},
  {"print",     &cmdPrint,    CMD_PRINT},
  {"run ",      &cmdRun,      CMD_RUN},
  {"sdi ",      &cmdSDI12,    CMD_SDI12},
  {"time gps",  &cmdTimeGPS,  CMD_TIME_GPS},
  {"time ",     &cmdTime,     CMD_TIME},
};

const uint8_t nCommands = sizeof commands / sizeof commands[0];


int8_t exeCommand(const char* value)
{
  Command cmd;
  int8_t error;
  size_t len;

  for (uint8_t i=0; i < nCommands; i++)
  {
    memcpy_P(&cmd, &commands[i], sizeof cmd);
    len = strlen(cmd.prefix);
    if (strncmp(cmd.prefix, value, len) == 0)
    {
      debug(F("command %s"), value);
      error = cmd.function(&(value[len]));
      if (error == 1)
      {
        warn(F("Bad input %s"), value);
      }
      return error;
    }
  }

  warn(F("Unexpected command %s"), value);
  return 1;
}

/**
 * Choose battery type.
 */
int8_t cmdBattery(const char* value)
{
  int value_int;

  // Check input
  if (sscanf(value, "%d", &value_int) != 1) { return 1; }
  if (value_int != 1 && value_int != 2) { return 1; }

  // Action
  UIO.batteryType = (uint8_t) value_int;
  UIO.updateEEPROM(EEPROM_UIO_BATTERY_TYPE, UIO.batteryType);
  debug(F("New battery type: %hu"), UIO.batteryType);
  return 0;
}

/**
 * Choose battery type.
 */
int8_t cmdCat(const char* value)
{
  char filename[80];

  // Check input
  if (sscanf(value, "%s", filename) != 1) { return 1; }
  if (strlen(filename) == 0) { return 1; }

  if (! UIO.hasSD)
  {
    cr.print(F("SD card not available"));
    return 2;
  }

  // Action
  SD.showFile((char*) filename);
  return 0;
}

/**
 * Enable/Disable features (flags).
 */

uint8_t _getFlag(const char* value)
{
  if (strncmp(value, "log_sd",  strlen("log_sd")) == 0)  return FLAG_LOG_SD;
  if (strncmp(value, "log_usb", strlen("log_usb")) == 0) return FLAG_LOG_USB;
  if (strncmp(value, "network", strlen("network")) == 0) return FLAG_NETWORK;

  return 0; // Not found
}

int8_t cmdDisable(const char* value)
{
  uint8_t flag;

  // Check input
  flag = _getFlag(value);
  if (flag == 0) { return 1; }

  // Action
  UIO.flags &= ~flag;
  UIO.updateEEPROM(EEPROM_UIO_FLAGS, UIO.flags);
  return 0;
}

int8_t cmdEnable(const char* value)
{
  uint8_t flag;

  // Check input
  flag = _getFlag(value);
  if (flag == 0) { return 1; }

  // Action
  UIO.flags |= flag;
  UIO.updateEEPROM(EEPROM_UIO_FLAGS, UIO.flags);
  return 0;
}

/**
 * Exit the command line interface
 */

int8_t cmdExit(const char* value)
{
  return -1;
}

/**
 * Format SD card.
 */

int8_t cmdFormat(const char* value)
{
  SD.format();
  return 0;
}

/**
 * Help: Prints the list of commands
 */

int8_t cmdHelp(const char* value)
{
  Command cmd;
  char help[120];

  for (uint8_t i=0; i < nCommands; i++)
  {
    memcpy_P(&cmd, &commands[i], sizeof cmd);
    strncpy_P(help, cmd.help, sizeof help);
    cr.print(F("%s"), help);
  }

  return 0;
}

/**
 * Sets log level.
 */

int8_t cmdLogLevel(const char* value)
{
  unsigned int loglevel;

  // Check input
  if (sscanf(value, "%u", &loglevel) != 1) { return 1; }
  if (loglevel >= LOG_LEN) { return 1; }

  // Action
  cr.loglevel = (loglevel_t) loglevel;
  UIO.updateEEPROM(EEPROM_UIO_LOG_LEVEL, (uint8_t) cr.loglevel);
  debug(F("New log level: %hu"), cr.loglevel);
  return 0;
}

/**
 * List files in SD.
 */

int8_t cmdLs(const char* value)
{
  SD.ls(LS_DATE | LS_SIZE | LS_R);
  return 0;
}

/**
 * Print configuration and other information
 */

int8_t cmdNetwork(const char* value)
{
  unsigned int network;

  // Check input
  if (sscanf(value, "%d", &network) != 1) { return 1; }
  if (network >= NETWORK_LEN) { return 1; }

  // Check availability
  if ((UIO.flags & FLAG_NETWORK) == 0)
  {
    warn(F("Network not available. First enable it with 'enable network'"));
    return 2;
  }

  // Action
  UIO.setNetwork((network_t) network);
  debug(F("New network: %d"), network);
  return 0;
}

/**
 * Print configuration and other information
 */

int8_t cmdPrint(const char* value)
{
  char buffer[150];
  size_t size = sizeof(buffer);

  cr.print(F("Time    : %s"), RTC.getTime());
  cr.print(F("Battery : %s (%d %%)"), UIO.menuFormatBattery(buffer, size), UIO.batteryLevel);
  cr.print(F("Log     : level=%s output=%s"), cr.loglevel2str(cr.loglevel), UIO.menuFormatLog(buffer, size));
  cr.print(F("Network : %s"), UIO.menuFormatNetwork(buffer, size));
  cr.print(F("Actions : %s"), UIO.menuFormatActions(buffer, size));

  return 0;
}

/**
 * Running things at time intervals.
 */

int8_t cmdRun(const char* value)
{
  unsigned int what;
  unsigned int minutes;

  // Check input
  if (sscanf(value, "%u %u", &what, &minutes) != 2) { return 1; }
  if (what >= RUN_LEN) { return 1; }
  if (minutes > 255) { return 1; }

  // Action
  UIO.actions[what] = (uint8_t) minutes;
  UIO.updateEEPROM(EEPROM_RUN + what, UIO.actions[what]);
  debug(F("Run %u every %u minutes"), what, UIO.actions[what]);
  return 0;
}

/**
 * SDI-12
 */

int8_t cmdSDI12(const char* value)
{
  // Action
  cr.print(F("Enabling SDI-12"));
  PWR.setSensorPower(SENS_5V, SENS_ON);
  mySDI12.begin();
  mySDI12.identification(0);
  mySDI12.identification(1);
  mySDI12.end();
  PWR.setSensorPower(SENS_5V, SENS_OFF);
  cr.print(F("Disabling SDI-12"));
  return 0;
}

/**
 * Sets time. Accepts two formats:
 *
 * - time=yy:mm:dd:hh:mm:ss
 * - time=<seconds since the epoch>
 *
 */

int8_t cmdTime(const char* value)
{
  unsigned short year, month, day, hour, minute, second;
  unsigned long epoch;
  timestamp_t time;

  if (sscanf(value, "%hu:%hu:%hu:%hu:%hu:%hu", &year, &month, &day, &hour, &minute, &second) == 6)
  {
    // time=yy:mm:dd:hh:mm:ss
  }
  else if (sscanf(value, "%lu", &epoch) == 1)
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
    return 1;
  }

  // Save
  if (UIO.saveTime(year, month, day, hour, minute, second) != 0)
  {
    error(F("Failed to save time %lu"), epoch);
    return 2;
  }

  // Success!!
  debug(F("New time: %s"), RTC.getTime());
  return 0;
}

/**
 * Sets time. Accepts two formats:
 *
 * - time=yy:mm:dd:hh:mm:ss
 * - time=<seconds since the epoch>
 *
 */

int8_t cmdTimeGPS(const char* value)
{
  uint32_t before, after;
  uint32_t start, time;

  if (! UIO.hasGPS)
  {
    warn(F("GPS not available"));
    return 2;
  }

  // On
  if (GPS.ON() == 0)
  {
    warn(F("GPS: GPS.ON() failed"));
    return 2;
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
    return 2;
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
    return 2;
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
  return 0;
}
