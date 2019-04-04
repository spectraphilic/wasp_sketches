#include "WaspUIO.h"


/**
 * Function to be called first in setup()
 */

void WaspUIO::boot()
{
  nloops = 0;
  SdFile::dateTimeCallback(WaspUIO::dateTime);

  RTC.ON();
  cr.print(F(".")); UIO.bootConfig();  // Read config from EEPROM
  cr.print(F(".")); UIO.bootDetect();  // Auto detect hardware
  cr.print(F(".")); UIO.onLoop();
  cr.print(F(".")); UIO.networkInit(); // Init network

  if (_boot_version < 'H')
  {
    cr.println(F("\nWARNING boot version=%c (only version H and above are supported)"), (char) _boot_version);
  }
}


void WaspUIO::bootConfig()
{
  // Flags
  flags = Utils.readEEPROM(EEPROM_UIO_FLAGS);

  // Network type
  networkType = (network_type_t) Utils.readEEPROM(EEPROM_UIO_NETWORK_TYPE);
  if (networkType >= NETWORK_LEN) { networkType = NETWORK_XBEE; }

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

#if WITH_XBEE
  // XBee network
  uint8_t panid_low = Utils.readEEPROM(EEPROM_UIO_XBEE+1);
  if (panid_low >= xbee_len) { panid_low = 2; } // Default
  memcpy_P(&xbee, &xbees[panid_low], sizeof xbee);
#endif

  // Read run table
  for (uint8_t i=0; i < RUN_LEN; i++)
  {
    const char* name = (const char*)pgm_read_word(&(run_names[i]));
    if (strcmp_P("", name) == 0)
    {
      actions[i] = 0;
    }
    else
    {
      uint16_t base = EEPROM_UIO_RUN + (i * 2);
      uint8_t hours = Utils.readEEPROM(base);
      uint8_t minutes = Utils.readEEPROM(base + 1);
      actions[i] = (hours * 60) + minutes;
    }
  }

  // Log level
  cr.loglevel = (loglevel_t) Utils.readEEPROM(EEPROM_UIO_LOG_LEVEL);

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
  readBattery();

  pinMode(PIN_1WIRE, INPUT);
  pinMode(PIN_SDI12, INPUT);

  startSD();
  loadTime();
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

uint32_t WaspUIO::nextAlarm()
{
  uint32_t next = ULONG_MAX / 60; // max posible value of minutes since epoch
  timestamp_t ts;

  // Minutes since epoch
  // With +2s safeguard, this means we've 2s before going to sleep
  uint32_t epoch = getEpochTime();
  uint32_t minutes = (epoch + 2) / 60;

  for (uint8_t i=0; i < RUN_LEN; i++)
  {
    uint32_t value = actions[i] * cooldown;
    if (value > 0)
    {
      value = (minutes / value + 1) * value;
      if (value < next)
      {
        next = value;
      }
    }
  }


  // Set Alarm-1 and Alarm-2 (watchdog)
  //
  // Use MODE3 instead of MODE2 to not sleep for more than 1 day, in case
  // somehow the RTC loses it's time but the alarms are not reset.
  //
  // Be paronoid and set Alarm-1 at 01s, because Alarm-2 can only run at 00s.
  // This way we are extra sure both are not triggered at the same time and we
  // get predictable results. This should not be necessary because we set
  // Alarm-2 2 minues later.
  //
  RTC.ON();
  RTC.breakTimeAbsolute(next * 60, &ts);
  RTC.setAlarm1(ts.date, ts.hour, ts.minute, 1, RTC_ABSOLUTE, RTC_ALM1_MODE3);
  info(F("Alarm 1 at %02d:%02d:%02d:%02d mode=3 (only hour/min/sec match)"), ts.date, ts.hour, ts.minute, 1);
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
    fatal(F("Error setting watchdog"));
    reboot();
  }
}



/*
 * Preinstantiate Objects.
 * We have to do it somewhere.
 */
WaspUIO UIO = WaspUIO();
FIFO fifo = FIFO("TMP.TXT", "QSTART.BIN", 8); // TODO Rename to FIFO.BIN / FIDX.BIN
#if WITH_IRIDIUM
IridiumSBD iridium(Serial1, DIGITAL4); // RING pins not connected
LIFO lifo = LIFO("LIFO.BIN", 8);
#endif
