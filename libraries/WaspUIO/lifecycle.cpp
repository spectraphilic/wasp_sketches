#include "WaspUIO.h"


int upgradeFIFO()
{
  if (SD.isFile("TMP.TXT") == -1)
  {
    return 0;
  }

  // Open old
  FIFO old = FIFO("TMP.TXT", "QSTART.BIN", 8);
  if (old.open(O_READ)) { return 1; } // Error
  int32_t nitems = old.len();
  cr_printf("Upgrading FIFO: %ld items (1 dot = 20 items)\n", nitems);

  // Create new (delete first in case previous upgrade was interrupted)
  SD.del("FIFO.BIN");
  SD.del("FIDX.BIN");
  fifo.make();

  // Upgrade
  const int progress_nth = 20; // Print a dot every .. items
  const int progress_nl = 60 * progress_nth; // Print a new line every .. dots

  fifo.open(O_RDWR | O_CREAT);
  uint8_t item[9] = {0};
  for (int32_t idx=0; idx < nitems; idx++)
  {
    if (old.peek(&item[1], idx) || fifo.push(item)) { goto fail; }

    // Progress
    if ((idx+1) % progress_nth == 0)
    {
      cr_printf(".");
      if ((idx+1) % progress_nl == 0)
      {
        if (fifo.sync()) { goto fail; }
        cr_printf("\n");
      }
    }
  }
  fifo.close();

  // Remove old
  old.close();
  SD.del("TMP.TXT");
  SD.del("QSTART.BIN");
  cr_printf("Done.\n");
  return 0;

fail:
  fifo.close();
  old.close();
  return 1;
}

#if WITH_IRIDIUM
int upgradeLIFO()
{
  if (SD.isFile("LIFO.BIN") == -1)
  {
    return 0;
  }

  // Open old
  LIFO old = LIFO("LIFO.BIN", 8);
  if (old.open(O_READ)) { return 1; } // Error
  int32_t nitems = old.len();
  cr_printf("Upgrading LIFO: %ld items (1 dot = 20 items)\n", nitems);

  // Create new (delete first in case previous upgrade was interrupted)
  SD.del("LIFO2.BIN");
  lifo.make();

  // Upgrade
  const int progress_nth = 20; // Print a dot every .. items
  const int progress_nl = 60 * progress_nth; // Print a new line every .. dots

  lifo.open(O_RDWR | O_CREAT);
  uint8_t item[9] = {0};
  for (int32_t idx=0; idx < nitems; idx++)
  {
    if (old.peek(&item[1], idx) || lifo.push(item)) { goto fail; }

    // Progress
    if ((idx+1) % progress_nth == 0)
    {
      cr_printf(".");
      if ((idx+1) % progress_nl == 0)
      {
        if (lifo.sync()) { goto fail; }
        cr_printf("\n");
      }
    }
  }
  lifo.close();

  // Remove old
  old.close();
  SD.del("LIFO.BIN");
  cr_printf("Done.\n");
  return 0;

fail:
  lifo.close();
  old.close();
  return 1;
}
#endif


/**
 * Function to be called first in setup()
 */

void WaspUIO::boot()
{
  Utils.setLED(LED0, LED_ON);
  nloops = 0;
  SdFile::dateTimeCallback(WaspUIO::dateTime);

  USB.ON();
  RTC.ON();
  cr_printf(".");

  // Read config from EEPROM
  UIO.bootConfig();
  cr_printf(".");

  UIO.bootDetect();
  cr_printf(".");

  UIO.networkInit();
  cr_printf(".");

  // Load time and start SD
  UIO.onLoop();
  cr_printf(".\n");

  // Now we can start logging
  log_info("Welcome to wsn");
  if (_boot_version < 'H')
  {
    log_warn("Old boot version found (%c), only version H and above are supported", _boot_version);
  }

  // Command line interface
  clint();

  // Upgrade queues
  if (UIO.hasSD)
  {
    if (upgradeFIFO()) { cr_printf("ERROR Upgrading FIFO\n"); }
#if WITH_IRIDIUM
    if (upgradeLIFO()) { cr_printf("ERROR Upgrading LIFO\n"); }
#endif
  }

  RTC.OFF();
  USB.OFF();
}


void WaspUIO::bootConfig()
{
  // Read device name from EEPROM into static variable
  Utils.getID(name);

  // Variables
  cr.loglevel = (loglevel_t) LOG_LEVEL;
  log_sd = Utils.readEEPROM(EEPROM_UIO_VARS + VAR_LOG_SD_IDX);
  log_usb = Utils.readEEPROM(EEPROM_UIO_VARS + VAR_LOG_USB_IDX);
  lan_type = (lan_type_t) Utils.readEEPROM(EEPROM_UIO_VARS + VAR_LAN_TYPE_IDX);
  lan_wait = Utils.readEEPROM(EEPROM_UIO_VARS + VAR_LAN_WAIT_IDX);
  wan_type = (wan_type_t) Utils.readEEPROM(EEPROM_UIO_VARS + VAR_WAN_TYPE_IDX);
  lora_addr = Utils.readEEPROM(EEPROM_UIO_VARS + VAR_LORA_ADDR_IDX);
  lora_mode = Utils.readEEPROM(EEPROM_UIO_VARS + VAR_LORA_MODE_IDX);
  lora_dst = Utils.readEEPROM(EEPROM_UIO_VARS + VAR_LORA_DST_IDX);
  xbee_network = Utils.readEEPROM(EEPROM_UIO_VARS + VAR_XBEE_NETWORK_IDX);

  // Defaults
  if (lan_type >= LAN_LEN) lan_type = LAN_DISABLED;
  if (wan_type >= WAN_LEN) wan_type = WAN_DISABLED;

#if WITH_4G
  // 4G network
  char apn[30];
  pin = eeprom_read_word((uint16_t*)EEPROM_UIO_PIN);
  UIO.readEEPROM(EEPROM_UIO_APN, apn, sizeof apn);
  _4G.set_APN(apn);
#endif

  // Read run table
  for (uint8_t i=0; i < RUN_LEN; i++)
  {
    const char* name = (const char*)pgm_read_word(&(run_names[i]));
    if (strcmp_P("", name) == 0)
    {
      actions[i].type = action_disabled;
    }
    else
    {
      uint16_t base = EEPROM_UIO_RUN + (i * 2);
      uint8_t minute = Utils.readEEPROM(base);
      uint8_t type = minute >> 6;
      minute = minute & 0b00111111;
      uint8_t hour = Utils.readEEPROM(base + 1);
      actions[i] = (Action){(action_t)type, hour, minute};
    }
  }

  // Frames
  setFrameSize();
}


void WaspUIO::bootDetect()
{
  /* GPS */
  hasGPS = GPS_NO;
#if WITH_4G
  if (_4G.ON() == 0)
  {
    hasGPS |= GPS_4G;
    _4G.OFF();
  }
#endif
#if WITH_GPS
  if (GPS.ON() == 1)
  {
    hasGPS |= GPS_YES;
    GPS.OFF();
  }
#endif

  // USB
  // Cannot be done with ATMega1281 but it can be with ATmega328P
}



/**
 * Function to be called in every loop (and on boot as well)
 */

void WaspUIO::onLoop()
{
  // This one is used only to know for how long the loop run
  _loop_start = millis();

  readBattery();

#if WITH_EXT_CHARGE
  // Switch ON/OFF external charge pin
  pinMode(PIN_POWER_EXT, OUTPUT);
  if (batteryLevel <= 75) {
    digitalWrite(PIN_POWER_EXT, HIGH);
  } else if (batteryLevel > 88) {
    digitalWrite(PIN_POWER_EXT, LOW);
  }
#endif

  // Data lines
  pinMode(PIN_1WIRE, INPUT);
  pinMode(PIN_SDI12, INPUT);

  // SD & time
  startSD();
  loadTime();

  // This is the reference value used to decide whether actions are run
  _epoch_minutes = _epoch / 60;
}



/**
 * These 3 functions define the process to sleep for the next loop
 */

void WaspUIO::reboot()
{
  // Stop stuff
  UIO.stopSD();
  PWR.switchesOFF(ALL_OFF);

  // Clear interruption flag & pin
  intFlag = 0;
  PWR.clearInterruptionPin();

  // Reboot
  PWR.reboot();
}

uint32_t WaspUIO::__nextAlarmHelper(
  uint32_t now,   // Current time (minutes since epoch)
  uint32_t size,  // Time interval (minutes or hours)
  uint8_t factor, // 1 if size is in minutes, 60 if size is in hours
  uint8_t offset, // 0 if size is in minutes, otherwise some minutes
  uint8_t step    // 0 or 1
)
{
  if (factor == 0) { return 0; } // Avoid "division by zero"

  uint32_t value = size * cooldown;
  value = ((now / factor) / value + step) * value;
  return value * factor + offset;
}

uint32_t WaspUIO::nextAlarm()
{
  uint32_t max = ULONG_MAX / 60; // max posible value of minutes since epoch
  uint32_t next = max;

  // Minutes since epoch
  // With +2s safeguard, this means we've 2s before going to sleep
  uint32_t epoch = getEpochTime();
  uint32_t minutes = (epoch + 2) / 60;

  for (uint8_t idx=0; idx < RUN_LEN; idx++)
  {
    Action action = actions[idx];
    uint32_t value = max;

    if (action.type == action_minutes)
    {
      value = __nextAlarmHelper(minutes, action.minute);
    }
    else if (action.type == action_hours)
    {
      value = __nextAlarmHelper(minutes, action.hour, 60, action.minute, 0);
      if (value <= minutes)
      {
        value = __nextAlarmHelper(minutes, action.hour, 60, action.minute, 1);
      }
    }

    if (value < next)
    {
      next = value;
    }
  }

  //
  // Set Alarm-1 and Alarm-2 (watchdog)
  //
  // Use MODE3 instead of MODE2 to not sleep for more than 1 day, in case
  // somehow the RTC loses it's time but the alarms are not reset.
  //

  // It may be 1 to be different from Alarm-2, but both are clearly different already, by 2 minutes
  timestamp_t ts;
  uint8_t second = 0;
  uint8_t error;

  RTC.ON();

  // Alarm-1
  RTC.breakTimeAbsolute(next * 60, &ts);
  error = RTC.setAlarm1(ts.date, ts.hour, ts.minute, second, RTC_ABSOLUTE, RTC_ALM1_MODE3);
  if (error) {
    log_error("Failed to set Alarm 1");
    reboot();
  } else {
    log_info("Alarm 1 at %02d:%02d:%02d:%02d mode=3 (only hour/min/sec match)",
             ts.date, ts.hour, ts.minute, second);
  }

  // Alarm-2
  RTC.breakTimeAbsolute((next + 2) * 60, &ts);
  error = RTC.setAlarm2(ts.date, ts.hour, ts.minute, RTC_ABSOLUTE, RTC_ALM2_MODE3);
  if (error) {
    log_error("Failed to set Alarm 2");
    reboot();
  } else {
    log_info("Alarm 2 at %02d:%02d:%02d mode=3 (only hour/min match)", ts.date, ts.hour, ts.minute);
  }

  RTC.OFF();

  return 1;
}

void WaspUIO::deepSleep()
{
  saveTimeToSD();

  // For robustness sake, reboot once in a while
  if (nloops >= MAX_LOOPS)
  {
    log_info("Rebooting after %u loops", nloops);
    reboot();
  }

  nextAlarm();  // Set next alarm
  UIO.stopSD(); // Stop SD, logging ends here

  // Clear interruption flag & pin
  intFlag = 0;
  PWR.clearInterruptionPin();

  // Power off and Sleep
  Utils.setLED(LED0, LED_OFF);
  Utils.setLED(LED1, LED_OFF);
  PWR.sleep(ALL_OFF);
  Utils.setLED(LED1, LED_ON);

  // Awake
  nloops++;                 // Next loop
  if (RTC.setWatchdog(LOOP_TIMEOUT))
  {
    // We cannot log because the SD is not started yet
    cr_printf("FATAL ERROR setting watchdog\n");
    reboot();
  }

  // Starts time and SD
  onLoop();

  char buffer[50];
  log_info("===== Loop %u battery=%s =====", UIO.nloops,
       UIO.pprintBattery(buffer, sizeof buffer));
}



/*
 * Preinstantiate Objects.
 * We have to do it somewhere.
 */
WaspUIO UIO = WaspUIO();
FIFO fifo = FIFO("FIFO.BIN", "FIDX.BIN", 9);
#if WITH_IRIDIUM
IridiumSBD iridium(Serial1, PIN_ISBD_SLEEP); // RING pins not connected
LIFO lifo = LIFO("LIFO2.BIN", 9);
#endif
