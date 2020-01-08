#include "WaspUIO.h"


int upgradeFIFO()
{
  if (SD.isFile("TMP.TXT") == -1)
  {
    return 0;
  }
  cr.println(F("Upgrading FIFO..."));

  FIFO old = FIFO("TMP.TXT", "QSTART.BIN", 8);
  //FIFO fifo = FIFO("FIFO.BIN", "FIDX.BIN", 9);

  // Upgrade
  uint8_t item[9] = {0};
  int idx, err = 1;

  for (idx=0; true; idx++)
  {
    // Read from old FIFO
    int status = old.peek(&item[1], idx);
    if (status == QUEUE_EMPTY || status == QUEUE_INDEX_ERROR) // Stop condition
    {
      err = 0;
      break;
    }
    if (status) { break; } // Error

    // Write to new FIFO
    if (fifo.push(item)) { break; }
  }

  if (err)
  {
    // Redo new
    SD.del("FIFO.BIN");
    SD.del("FIDX.BIN");
    fifo.make();

    cr.println(F("ERROR Upgrading"));
    return 1;
  }

  // Remove old
  SD.del("TMP.TXT");
  SD.del("QSTART.BIN");

  return 0;
}

/**
 * Function to be called first in setup()
 */

void WaspUIO::boot()
{
  nloops = 0;
  SdFile::dateTimeCallback(WaspUIO::dateTime);

  USB.ON();
  RTC.ON();
  cr.print(F("."));

  // Read config from EEPROM
  UIO.bootConfig();
  cr.print(F("."));

  UIO.bootDetect();
  cr.print(F("."));

  UIO.networkInit();
  cr.print(F("."));

  // Load time and start SD
  UIO.onLoop();
  cr.println(F("."));

  // Now we can start logging
  info(F("Welcome to wsn"));
  if (_boot_version < 'H')
  {
    warn(F("Old boot version found (%c), only version H and above are supported"), _boot_version);
  }

  // Upgrade queues
  if (UIO.hasSD)
  {
    upgradeFIFO();
#if WITH_IRIDIUM
    upgradeLIFO();
#endif
  }

  // Command line interface
  clint();
  RTC.OFF();
  USB.OFF();
}


void WaspUIO::bootConfig()
{
  // Read device name from EEPROM into static variable
  Utils.getID(name);

  // Variables
  batteryType = (battery_type_t)Utils.readEEPROM(EEPROM_UIO_VARS + VAR_BAT_IDX);
  boardType = (board_type_t)Utils.readEEPROM(EEPROM_UIO_VARS + VAR_BOARD_IDX);
  cr.loglevel = (loglevel_t) Utils.readEEPROM(EEPROM_UIO_VARS + VAR_LOG_LEVEL_IDX);
  log_sd = Utils.readEEPROM(EEPROM_UIO_VARS + VAR_LOG_SD_IDX);
  log_usb = Utils.readEEPROM(EEPROM_UIO_VARS + VAR_LOG_USB_IDX);
  lan_type = (lan_type_t) Utils.readEEPROM(EEPROM_UIO_VARS + VAR_LAN_TYPE_IDX);
  wan_type = (wan_type_t) Utils.readEEPROM(EEPROM_UIO_VARS + VAR_WAN_TYPE_IDX);
  lora_addr = Utils.readEEPROM(EEPROM_UIO_VARS + VAR_LORA_ADDR_IDX);
  lora_mode = Utils.readEEPROM(EEPROM_UIO_VARS + VAR_LORA_MODE_IDX);
  xbee_network = Utils.readEEPROM(EEPROM_UIO_VARS + VAR_XBEE_NETWORK_IDX);
  xbee_wait = Utils.readEEPROM(EEPROM_UIO_VARS + VAR_XBEE_WAIT_IDX);

  // Defaults for safety
  if (batteryType >= BATTERY_LEN) { batteryType = BATTERY_LITHIUM; }
  if (boardType >= BOARD_LEN) { boardType = BOARD_NONE; }

#if WITH_CRYPTO
  // Frame encryption
  UIO.readEEPROM(EEPROM_UIO_PWD, password, sizeof password);
  size_t len = strlen(password);
  if (len != 0 && len != 16 && len != 24 && len != 32)
  {
    password[0] = '\0';
  }
#endif

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

  pinMode(PIN_1WIRE, INPUT);
  pinMode(PIN_SDI12, INPUT);

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

  RTC.ON();
  RTC.breakTimeAbsolute(next * 60, &ts);
  RTC.setAlarm1(ts.date, ts.hour, ts.minute, second, RTC_ABSOLUTE, RTC_ALM1_MODE3);
  info(F("Alarm 1 at %02d:%02d:%02d:%02d mode=3 (only hour/min/sec match)"), ts.date, ts.hour, ts.minute, second);
  // Alarm-2
  RTC.breakTimeAbsolute((next + 2) * 60, &ts);
  RTC.setAlarm2(ts.date, ts.hour, ts.minute, RTC_ABSOLUTE, RTC_ALM2_MODE3);
  info(F("Alarm 2 at %02d:%02d:%02d mode=3 (only hour/min match)"), ts.date, ts.hour, ts.minute);
  RTC.OFF();

  return 1;
}

void WaspUIO::deepSleep()
{
  saveTimeToSD();

  // For robustness sake, reboot once in a while
  if (nloops >= MAX_LOOPS)
  {
    info(F("Rebooting after %u loops"), nloops);
    reboot();
  }

  nextAlarm();  // Set next alarm
  UIO.stopSD(); // Stop SD, logging ends here

  // Clear interruption flag & pin
  intFlag = 0;
  PWR.clearInterruptionPin();

  // Power off and Sleep
  PWR.sleep(ALL_OFF);

  // Awake
  nloops++;                 // Next loop
  if (RTC.setWatchdog(LOOP_TIMEOUT))
  {
    // We cannot log because the SD is not started yet
    cr.println(F("FATAL ERROR setting watchdog"));
    reboot();
  }

  // Starts time and SD
  onLoop();

  char buffer[50];
  info(F("===== Loop %u battery=%s ====="), UIO.nloops,
       UIO.pprintBattery(buffer, sizeof buffer));
}



/*
 * Preinstantiate Objects.
 * We have to do it somewhere.
 */
WaspUIO UIO = WaspUIO();
FIFO fifo = FIFO("FIFO.BIN", "FIDX.BIN", 9);
#if WITH_IRIDIUM
IridiumSBD iridium(Serial1, DIGITAL4); // RING pins not connected
LIFO lifo = LIFO("LIFO2.BIN", 9);
#endif
