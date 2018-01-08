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

const char CMD_HELP         [] PROGMEM = "help           - Prints the list of commands";
const char CMD_PRINT        [] PROGMEM = "print          - Print configuration and other information";
const char CMD_SET_LOG_LEVEL[] PROGMEM = "loglevel=VALUE - Sets the log level: 0=off 1=fatal 2=error 3=warn 4=info 5=debug 6=trace";
const char CMD_SET_LOG_SD   [] PROGMEM = "logsd=VALUE    - Enables (1) or disables (0) logging to SD";
const char CMD_SET_LOG_USB  [] PROGMEM = "logusb=VALUE   - Enables (1) or disables (0) logging to USB";
const char CMD_SET_TIME_GPS [] PROGMEM = "time=gps       - Sets time from GPS";
const char CMD_SET_TIME     [] PROGMEM = "time=VALUE     - Sets time to the given value, format is yy:mm:dd:hh:mm:ss";

const Command commands[] PROGMEM = {
  {"help",      &cmdHelp,        CMD_HELP},
  {"print",     &cmdPrint,       CMD_PRINT},
  {"loglevel=", &cmdSetLogLevel, CMD_SET_LOG_LEVEL},
  {"logsd=",    &cmdSetLogSD,    CMD_SET_LOG_SD},
  {"logusb=",   &cmdSetLogUSB,   CMD_SET_LOG_USB},
  {"time=gps",  &cmdSetTimeGPS,  CMD_SET_TIME_GPS},
  {"time=",     &cmdSetTime,     CMD_SET_TIME},
};

const uint8_t nCommands = sizeof commands / sizeof commands[0];


int8_t exeCommand(const char* value)
{
  Command cmd;
  int8_t error;

  for (uint8_t i=0; i < nCommands; i++)
  {
    memcpy_P(&cmd, &commands[i], sizeof cmd);
    if (strncmp(cmd.prefix, value, strlen(cmd.prefix)) == 0)
    {
      debug(F("command %s"), value);
      error = cmd.function(value);
      if (error == 1)
      {
        warn(F("Bad input %s"), value);
      }
      return error;
    }
  }

  warn(F("Unexpected command %s"), value);
  return -1;
}

/**
 * Prints the list of commands
 */

int8_t cmdHelp(const char* value)
{
  Command cmd;
  char help[99];

  for (uint8_t i=0; i < nCommands; i++)
  {
    memcpy_P(&cmd, &commands[i], sizeof cmd);
    strncpy_P(help, cmd.help, sizeof help);
    cr.print(F("%s"), help);
  }

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
  cr.print(F("Log     : level=%s output=%s"), cr.loglevel2str(cr.loglevel), UIO.menuFormatLog(buffer, size));
/*
  cr.print(F("Log level  : %s"), cr.loglevel2str(cr.loglevel));
  cr.print(F("Log to SD  : %s"), flagStatus(FLAG_LOG_SD));
  cr.print(F("Log to USB : %s"), flagStatus(FLAG_LOG_USB));
*/

  return 0;
}

/**
 * Sets log level.
 */

int8_t cmdSetLogLevel(const char* value)
{
  int loglevel;

  // Parse input
  if (sscanf(value, "loglevel=%d", &loglevel) != 1) { return 1; }
  if ((loglevel < LOG_OFF) || (loglevel > LOG_TRACE)) { return 1; }

  // Action
  cr.loglevel = (loglevel_t) loglevel;
  UIO.updateEEPROM(EEPROM_UIO_LOG_LEVEL, (uint8_t) cr.loglevel);
  debug(F("New log level: %hu"), loglevel);
  return 0;
}

int8_t cmdSetLogSD(const char* value)
{
  int yes;

  // Parse input
  if (sscanf(value, "logsd=%d", &yes) != 1) { return 1; }
  if (yes != 0 && yes != 1) { return 1; }

  // Action
  if (yes)
  {
    UIO.flags |= FLAG_LOG_SD;
  }
  else
  {
    UIO.flags &= ~FLAG_LOG_SD;
  }
  UIO.updateEEPROM(EEPROM_UIO_FLAGS, UIO.flags);
  debug(F("New log to SD: %d"), yes);
  return 0;
}

int8_t cmdSetLogUSB(const char* value)
{
  int yes;

  // Parse input
  if (sscanf(value, "logsd=%d", &yes) != 1) { return 1; }
  if (yes != 0 && yes != 1) { return 1; }

  // Action
  if (yes)
  {
    UIO.flags |= FLAG_LOG_USB;
  }
  else
  {
    UIO.flags &= ~FLAG_LOG_USB;
  }
  UIO.updateEEPROM(EEPROM_UIO_FLAGS, UIO.flags);
  debug(F("New log to USB: %d"), yes);
  return 0;
}

/**
 * Sets time. Accepts two formats:
 *
 * - time=yy:mm:dd:hh:mm:ss
 * - time=<seconds since the epoch>
 *
 */

int8_t cmdSetTime(const char* value)
{
  unsigned short year, month, day, hour, minute, second;
  unsigned long epoch;
  timestamp_t time;

  if (sscanf(value, "time=%hu:%hu:%hu:%hu:%hu:%hu", &year, &month, &day, &hour, &minute, &second) == 6)
  {
    // time=yy:mm:dd:hh:mm:ss
  }
  else if (sscanf(value, "time=%lu", &epoch) == 1)
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

int8_t cmdSetTimeGPS(const char* value)
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
