/*
 * The signature of commands is:
 *
 *   int8_t cmd(const char* command);
 *
 * Arguments:
 *
 *   command: The command to execute
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
  cmd_status_t (*function)(const char *command);
  const char* help;
} Command;

const char CAT_MAIN      [] PROGMEM = "\nMain commands:";
const char CMD_EXIT      [] PROGMEM = "  exit               - Exit the command line interface";
const char CMD_HELP      [] PROGMEM = "  help               - Print the commands list";
const char CMD_INFO      [] PROGMEM = "  info               - Print configuration and other information";
const char CMD_NAME      [] PROGMEM = "  name               - Give a name to the mote (max 16 chars)";
const char CMD_REBOOT    [] PROGMEM = "  reboot             - Reboot waspmote";
const char CMD_RUN       [] PROGMEM = "  run [NAME H[:MM]]  - run (list names), run name 0 (disable), "
                                      "run name 5 (every 5m), run name 1:07 (every 1h at :07)";
const char CMD_TIME      [] PROGMEM = "  time VALUE         - Set time, value can be 'network', 'gps', "
                                      "yy:mm:dd:hh:mm:ss or epoch";
const char CMD_VAR       [] PROGMEM = "  var [NAME [VALUE]] - type 'var' to list the variable names";
const char CAT_FS        [] PROGMEM = "\nFilesystem commands:";
const char CMD_CAT       [] PROGMEM = "  cat FILENAME       - Print FILENAME contents to USB";
const char CMD_CATX      [] PROGMEM = "  catx FILENAME      - Print FILENAME contents in hexadecimal to USB";
const char CMD_FORMAT    [] PROGMEM = "  format             - Format SD";
const char CMD_LS        [] PROGMEM = "  ls                 - List files in SD";
const char CMD_RM        [] PROGMEM = "  rm FILENAME        - Remove file";
const char CMD_TAIL      [] PROGMEM = "  tail N FILENAME    - Print last N lines of FILENAME to USB";
const char CAT_SENSORS   [] PROGMEM = "\nSensor commands:";
const char CMD_1WIRE_READ[] PROGMEM = "  1wire read         - Read DS18B20 string";
const char CMD_1WIRE_SCAN[] PROGMEM = "  1wire scan PIN+    - Scan DS18B20 in the given pins, save to onewire.txt";
const char CMD_I2C       [] PROGMEM = "  i2c [NAME]         - Scan I2C bus or read values from NAME: "
                                      "as7263 as7265 bme bm76 mlx tmp vl";
const char CMD_MB        [] PROGMEM = "  mb                 - Read the MB7389";
const char CMD_SDI12     [] PROGMEM = "  sdi COMMAND        - examples: ? (query) 5I (identity) 5A4 (change address)";
const char CAT_NETWORK   [] PROGMEM = "\nNetwork commands:";
const char CMD_4G_APN    [] PROGMEM = "  4g apn [APN]       - Set 4G Access Point Name";
const char CMD_4G_GPS    [] PROGMEM = "  4g gps             - Get position from 4G's GPS";
const char CMD_4G_PIN    [] PROGMEM = "  4g pin VALUE       - Set pin for the 4G module (0=disabled)";
const char CMD_LORA      [] PROGMEM = "  lora               - Print Lora info";
const char CMD_PING      [] PROGMEM = "  ping               - Network test";
const char CAT_OTHER     [] PROGMEM = "\nOther commands:";
const char CMD_GPS       [] PROGMEM = "  gps                - Get position from GPS";

const Command commands[] PROGMEM = {
    // Main commands
    {"",              NULL,            CAT_MAIN},
    {"exit",          &cmdExit,        CMD_EXIT},
    {"help",          &cmdHelp,        CMD_HELP},
    {"info",          &cmdInfo,        CMD_INFO},
    {"name",          &cmdName,        CMD_NAME},
    {"reboot",        &cmdReboot,      CMD_REBOOT},
    {"run",           &cmdRun,         CMD_RUN},
    {"time ",         &cmdTime,        CMD_TIME},
    {"var",           &cmdVar,         CMD_VAR},
    {"",              NULL,            CAT_FS},
    {"cat ",          &cmdCat,         CMD_CAT},
    {"catx ",         &cmdCatx,        CMD_CATX},
    {"format",        &cmdFormat,      CMD_FORMAT},
    {"ls",            &cmdLs,          CMD_LS},
    {"rm ",           &cmdRm,          CMD_RM},
    {"tail",          &cmdTail,        CMD_TAIL},
    // Sensors
#if WITH_1WIRE || WITH_I2C || WITH_MB || WITH_SDI
    {"",              NULL,            CAT_SENSORS},
    #if WITH_1WIRE
    {"1wire read",    &cmd1WireRead,   CMD_1WIRE_READ},
    {"1wire scan ",   &cmd1WireScan,   CMD_1WIRE_SCAN},
    #endif
    #if WITH_I2C
    {"i2c",           &cmdI2C,         CMD_I2C},
    #endif
    #if WITH_MB
    {"mb",            &cmdMB,          CMD_MB},
    #endif
    #if WITH_SDI
    {"sdi",           &cmdSDI12,       CMD_SDI12},
    #endif
#endif
    // Network
    {"",              NULL,            CAT_NETWORK},
    #if WITH_4G
    {"4g apn",        &cmd4G_APN,      CMD_4G_APN},
    {"4g gps",        &cmd4G_GPS,      CMD_4G_GPS},
    {"4g pin ",       &cmd4G_Pin,      CMD_4G_PIN},
    #endif
    #if WITH_LORA
    {"lora",          &cmdLora,        CMD_LORA},
    #endif
    {"ping",          &cmdPing,        CMD_PING},
    // Other
#if WITH_GPS
    {"",              NULL,            CAT_OTHER},
    {"gps",           &cmdGPS,         CMD_GPS},
#endif
    // Internal use only
    {"ack",           &cmdAck,         EMPTY_STRING},
};

const uint8_t nCommands = sizeof commands / sizeof commands[0];


/*
 * Command Line Interpreter (cli)
 */

void WaspUIO::clint()
{
  char buffer[180];
  size_t size = sizeof(buffer);

  // Print info
  cr_printf("\n");
  cmdInfo(NULL);
  cr_printf("\n");

  // Go interactive or not
  cr_printf("Press Enter to start interactive mode. Wait 2 seconds to skip.\n");
  if (cr.input(buffer, sizeof(buffer), 2000) != NULL)
  {
    cr_printf("Type 'help' for the commands list. Prompt timeouts after 3min of inactivity.\n");
    do {
      cr_printf("> ");
      if (cr.input(buffer, size, 3 * 60 * 1000UL) == NULL)
      {
        cr_printf("\nTimeout!\n");
        break;
      }

      USB.println(buffer);
      if (exeCommands(buffer, true) == cmd_exit)
      {
        break;
      }
    } while (true);
  }
  else
  {
    cr_printf("Timeout.\n");
  }

  cr_printf("\n");
}


/*
 * Call command
 */

cmd_status_t exeCommands(const char *str, bool interactive)
{
  cmd_status_t status = cmd_bad_input;
  const char *p = str;

  while (p != NULL)
  {
    // Find next command if any (commands separated by semicolon)
    char *n = strchr(p, ';');
    if (n != NULL)
    {
      *n = '\0';
      n++;
    }

    if (interactive)
    {
      status = exeCommand(p);
      if      (status == cmd_bad_input)   { cr_printf("I don't understand\n"); }
      else if (status == cmd_unavailable) { cr_printf("Feature not available\n"); }
      else if (status == cmd_error)       { cr_printf("Error\n"); }
      else if (status == cmd_ok)          { cr_printf("OK\n"); }
      else if (status == cmd_quiet)       { }
      else if (status == cmd_exit)        { cr_printf("Good bye!\n"); break; }
    } else {
      log_info("Command \"%s\"", p);
      status = exeCommand(p);
    }

    // Next command
    p = n;
  }

  return status;
}

COMMAND(exeCommand)
{
    Command cmd;
    cmd_status_t status;

    for (uint8_t i=0; i < nCommands; i++) {
        memcpy_P(&cmd, &commands[i], sizeof cmd);
        size_t len = strlen(cmd.prefix);
        if (len > 0 && strncmp(cmd.prefix, str, len) == 0) {
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
  if (UIO.ack_wait == 0)
  {
    log_warn("unexpected ack command");
    return cmd_quiet;
  }

#if WITH_IRIDIUM
  if (lifo.drop_end(UIO.ack_wait)) { return cmd_error; }
#else
  if (fifo.drop_begin(UIO.ack_wait)) { return cmd_error; }
#endif

#if WITH_LORA
  UIO.lora_fails = 0;
#endif

  UIO.ack_wait = 0; // Ready for next frame!
  return cmd_ok;
}

/**
 * Exit the command line interface
 */

COMMAND(cmdExit)
{
  return cmd_exit;
}

#if WITH_GPS
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
  if ((UIO.hasGPS & GPS_YES) == 0) { return cmd_unavailable; }

  if (UIO.gps(false, true) == -1)
  {
    return cmd_error;
  }

  return cmd_ok;
}
#endif

/**
 * Help: Prints the list of commands
 */

COMMAND(cmdHelp)
{
    Command cmd;
    char help[130];

    for (uint8_t i=0; i < nCommands; i++) {
        memcpy_P(&cmd, &commands[i], sizeof cmd);
        strncpy_P(help, cmd.help, sizeof help);
        if (strlen(help) > 0) { // Do not print help for hidden commands
            USB.println(help);
        }
    }

    return cmd_quiet;
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
 * Print configuration and other information
 */

COMMAND(cmdInfo)
{
  char name[17];
  char buffer[150];
  size_t size = sizeof(buffer);

  Utils.getID(name);

  cr_printf("Time     : %s\n", UIO.pprintTime(buffer, size));
  cr_printf("Id       : %s Version=%c Name=%s\n", UIO.pprintSerial(buffer, size), _boot_version, name);
  cr_printf("Battery  : %s\n", UIO.pprintBattery(buffer, size));
  cr_printf("Hardware : board=%s SD=%d GPS=%d\n", UIO.pprintBoard(buffer, size), UIO.hasSD, UIO.hasGPS);
#if WITH_XBEE
  if (UIO.lan_type == LAN_XBEE)
    cr_printf("XBee     : %s\n", UIO.pprintXBee(buffer, size));
#endif
#if WITH_LORA
  if (UIO.lan_type == LAN_LORA)
    cr_printf("Lora     : %s\n", UIO.pprintLora(buffer, size));
#endif
#if WITH_4G
  if (UIO.wan_type == WAN_4G)
    cr_printf("4G        : %s\n", UIO.pprint4G(buffer, size));
#endif
#if WITH_IRIDIUM
  if (UIO.wan_type == WAN_IRIDIUM)
    cr_printf("Iridium  : %s\n", UIO.pprintIridium(buffer, size));
#endif
  cr_printf("Frames   : %s\n", UIO.pprintFrames(buffer, size));
  cr_printf("Log      : level=%s output=%s\n", cr.loglevel2str(cr.loglevel), UIO.pprintLog(buffer, size));
  cr_printf("Run      : %s\n", UIO.pprintActions(buffer, size));

  return cmd_quiet;
}


COMMAND(cmdReboot)
{
  cr_printf("Waspmote will reboot in 2s...\n");
  delay(2000);

  UIO.reboot();
  return cmd_ok;
}

/**
 * Running things at time intervals.
 */

COMMAND(cmdRun)
{
  char name[11];
  uint8_t hour, minute;

  // Check input
  int n = sscanf(str, "%10s %hhu:%hhu", name, &hour, &minute);

  // Print names
  if (n == -1)
  {
    for (uint8_t i=0; i < RUN_LEN; i++)
    {
      const char* xname = (const char*)pgm_read_word(&(run_names[i]));
      if (strcmp_P("", xname) == 0)
        continue;

      // avr-libc uses %S for string stored in memory, this is very useful. But
      // GCC uses %S for wide char strings, and so emits a warning for the line
      // below. We silence this warning using cr.printf_P instead of the
      // cr_printf macro.
      cr.printf_P(PSTR("%S\n"), xname);
    }
    return cmd_quiet;
  }

  if (n < 2 || n > 3)
  {
    return cmd_bad_input;
  }

  int8_t value = UIO.index(run_names, sizeof run_names / sizeof run_names[0], name);
  if (value == -1) { return cmd_bad_input; }

  action_t type;
  if (n == 2)
  {
    type = action_minutes;
    minute = hour;
    hour = 0;
    if (minute == 0)
    {
      type = action_disabled;
    }
  }
  else
  {
    type = action_hours;
    if (hour == 0) { return cmd_bad_input; }
  }

  if (minute > 59) { return cmd_bad_input; }
  UIO.actions[value] = (Action){type, hour, minute};

  uint16_t base = EEPROM_UIO_RUN + (value * 2);
  minute = minute | (type << 6); // Pack the type within the same byte as the minute
  if (! UIO.updateEEPROM(base, minute)) { return cmd_error; }
  if (! UIO.updateEEPROM(base + 1, hour)) { return cmd_error; }

  return cmd_ok;
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
    log_error("Failed to save time %lu", epoch);
    return cmd_error;
  }

  return cmd_ok;
}


/**
 * Set variable value.
 */

COMMAND(cmdVar)
{
  // Number of variables
  uint8_t nvars = sizeof var_names / sizeof var_names[0];
  // Input data
  char name[16];
  int n;
  uint8_t value;

  // Read input
  n = sscanf(str, "%15s %hhu", name, &value);
  if (n == -1)
  {
    for (uint8_t i=0; i < nvars; i++)
    {
      const char* xname = (const char*)pgm_read_word(&(var_names[i]));
      if (strcmp_P("", xname) == 0)
        continue;

      const char* xhelp = (const char*)pgm_read_word(&(var_help[i]));
      cr.printf_P(PSTR("%S %S\n"), xname, xhelp);
    }
    return cmd_quiet;
  }

  if (n < 1 || n > 2)
  {
    return cmd_bad_input;
  }

  // Action
  int8_t idx = UIO.index(var_names, nvars, name);
  switch (idx)
  {
    case -1:
      return cmd_bad_input;
    case VAR_LOG_SD_IDX:
      if (n == 1) {
        value = UIO.log_sd;
      } else {
        if (value > 1) { return cmd_bad_input; }
        UIO.log_sd = value;
      }
      break;
    case VAR_LOG_USB_IDX:
      if (n == 1) {
        value = UIO.log_usb;
      } else {
        if (value > 1) { return cmd_bad_input; }
        UIO.log_usb = value;
      }
      break;
    case VAR_LAN_TYPE_IDX:
      if (n == 1) {
        value = (uint8_t)UIO.lan_type;
      } else {
        if (value >= LAN_LEN) { return cmd_bad_input; }
        UIO.lan_type = (lan_type_t)value;
        UIO.setFrameSize();
      }
      break;
    case VAR_WAN_TYPE_IDX:
      if (n == 1) {
        value = (uint8_t)UIO.wan_type;
      } else {
        if (value >= WAN_LEN) { return cmd_bad_input; }
        UIO.wan_type = (wan_type_t)value;
        UIO.setFrameSize();
      }
      break;
    case VAR_LORA_ADDR_IDX:
      if (n == 1) {
        value = UIO.lora_addr;
      } else {
        if (value == 0) { return cmd_bad_input; }
        UIO.lora_addr = value;
      }
      break;
    case VAR_LORA_MODE_IDX:
      if (n == 1) {
        value = UIO.lora_mode;
      } else {
        if (value == 0 || value > 10) { return cmd_bad_input; }
        UIO.lora_mode = value;
      }
      break;
    case VAR_XBEE_NETWORK_IDX:
      if (n == 1) {
        value = UIO.xbee_network;
      } else {
        if (value >= xbee_len) { return cmd_bad_input; }
        UIO.xbee_network = value;
#if WITH_XBEE
        UIO.xbeeInit();
#endif
      }
      break;
    case VAR_LAN_WAIT_IDX:
      if (n == 1) {
        value = UIO.lan_wait;
      } else {
        UIO.lan_wait = value;
      }
      break;
    case VAR_LORA_DST_IDX:
      if (n == 1) {
        value = UIO.lora_dst;
      } else {
        UIO.lora_dst = value;
      }
      break;
    default:
      return cmd_bad_input;
  }

  // Update
  if (n == 2)
  {
    if (! UIO.updateEEPROM(EEPROM_UIO_VARS + idx, value))
    {
      return cmd_error;
    }
  }

  // Print
  cr_printf("%u\n", value);
  return cmd_quiet;
}
