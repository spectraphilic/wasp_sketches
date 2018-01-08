#include "WaspUIO.h"

typedef struct {
  char prefix[12];
  int8_t (*function)(const char* command);
} Command;

const Command commands[] PROGMEM = {
  {"time=", &cmdTime},
};

const uint8_t nCommands = sizeof commands / sizeof commands[0];


int8_t exeCommand(const char* value)
{
  Command cmd;

  for (uint8_t i=0; i < nCommands; i++)
  {
    memcpy_P(&cmd, &commands[i], sizeof cmd);
    if (strncmp(cmd.prefix, value, strlen(cmd.prefix)) == 0)
    {
      debug(F("command %s"), value);
      return cmd.function(value);
    }
  }

  warn(F("Unexpected command %s"), value);
  return -1;
}

/**
 * Function to receive, parse and update RTC from GPS_sync frame
 *
 * Parameters: void
 *
 * Returns: uint8_t (0 on success, 1 on error)
 */

int8_t cmdTime(const char* cmd)
{
  unsigned short year, month, day, hour, minute, second;
  unsigned long epoch;
  timestamp_t time;

  if (sscanf(cmd, "time=%hu:%hu:%hu:%hu:%hu:%hu", &year, &month, &day, &hour, &minute, &second) == 6)
  {
    // time=yy:mm:dd:hh:mm:ss
  }
  else if (sscanf(cmd, "time=%lu", &epoch) == 1)
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
    warn(F("Bad input %s"), cmd);
    return 1;
  }

  // Save
  if (UIO.saveTime(year, month, day, hour, minute, second) != 0)
  {
    error(F("Failed to save time %lu"), epoch);
    return 1;
  }

  // Success!!
  debug(F("New time is %s"), RTC.getTime());
  return 0;
}
