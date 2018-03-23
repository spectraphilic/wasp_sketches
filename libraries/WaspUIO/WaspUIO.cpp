/******************************************************************************
 * Includes
 ******************************************************************************/

#include "WaspUIO.h"
#include <assert.h>


/******************************************************************************
* Constructors
******************************************************************************/

SDI12 mySDI12(PIN_SDI12);


/******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * Function to be called first in setup()
 */
void WaspUIO::onSetup()
{
  /*** 1. Read configuration from EEPROM ***/

  // Flags
  flags = Utils.readEEPROM(EEPROM_UIO_FLAGS);

  boardType = (board_type_t) Utils.readEEPROM(EEPROM_UIO_BOARD_TYPE);
  if (boardType >= BOARD_LEN) { boardType = BOARD_NONE; }

  batteryType = (battery_type_t) Utils.readEEPROM(EEPROM_UIO_BATTERY_TYPE);
  if (batteryType >= BATTERY_LEN) { batteryType = BATTERY_LITHIUM; }

  // Network
  uint8_t panid_low = Utils.readEEPROM(EEPROM_UIO_NETWORK+1);
  if (panid_low >= network_len)
  {
    panid_low = 2; // Default
  }
  memcpy_P(&network, &networks[panid_low], sizeof network);

  // Read run table
  for (uint8_t i=0; i < RUN_LEN; i++)
  {
    actions[i] = Utils.readEEPROM(EEPROM_RUN + i);
  }

  // Log level
  cr.loglevel = (loglevel_t) Utils.readEEPROM(EEPROM_UIO_LOG_LEVEL);

  /*** 2. Autodetect hardware ***/
  hasSD = SD.ON();
  SD.OFF();
  hasGPS = GPS.ON();
  GPS.OFF();

/*
    // USB autodetect
    debug(F("USB before %d"), serialAvailable(0));
    USB.ON();
    uint8_t usb = serialAvailable(0);
    USB.OFF();
    debug(F("USB after %d"), usb);
*/
}

void WaspUIO::onLoop()
{
  cr.sleep_time = 0;
  state = 0;

  loadTime(true); // Read temperature as well
  uint32_t epoch = getEpochTime();
  // Split the epoch time in 2: days since the epoch, and minute of the day
  day = epoch / (24L * 60L * 60L);
  minute = (epoch / 60) % (60 * 24); // mins since the epoch modulus mins in a day

  readBattery();
}


/**
 * Functions to start and stop SD. Open and closes required files.
 */

void WaspUIO::startSD()
{
  if (hasSD)
  {
    if (! (WaspRegister & REG_SD))
    {
      SD.ON();
      baselayout();
    }
  }
}

void WaspUIO::stopSD()
{
  if (SPI.isSD)
  {
    // Close files
    if (tmpFile.isOpen()) { tmpFile.close(); }
    if (logFile.isOpen()) { logFile.close(); }
    // Off
    SD.OFF();
  }
}


/**
 * Wrap the frame.addSensor functions. If there is no place left in the frame,
 * aves the frame to the SD, creates a new one, and then adds the sensor to the
 * new frame.
 */

void WaspUIO::createFrame(bool discard)
{
  if (frame.numFields > 1 && !discard)
  {
    frame2Sd();
  }

  frame.createFrameBin(BINARY);

  // In binary frames, the timestamp must be first, that's what I deduce from
  // frame.addTimestamp
  frame.addSensorBin(SENSOR_TST, epochTime);
}


/**
 * This function:
 * - Stores the frame in the "archive file"
 * - Appends the to the LIFO queue (tmp file), to be sent later
 *
 * The name of the archive file is "YYMMDD.TXT". This means that every frame
 * can be referenced with 7 bytes: 3 for the date, and 4 for the offset within
 * the archive file.
 *
 * Every entry in the LIFO queue (tmp file) is 7 bytes long:
 * - 0: year
 * - 1: month
 * - 2: date
 * - 3-6: offset in the archive file
 *
 * Parameters:
 * - char* filename: Filename of the LIFO queue (tmp file)
 *
 * Returns: uint8_t
 * - 0 success
 * - 1 error, failed to archive frame
 * - 2 error, archived frame, but append to queue failed
 */

uint8_t WaspUIO::frame2Sd()
{
  uint32_t size;
  uint8_t item[8];
  char dataFilename[18]; // /data/YYMMDD.txt
  SdFile dataFile;

  if (! hasSD)
  {
    return 1;
  }
  startSD();

  // (1) Get the date
  item[0] = RTC.year;
  item[1] = RTC.month;
  item[2] = RTC.date;
  getDataFilename(dataFilename, item[0], item[1], item[2]);

  // (2) Store frame in archive file
  if (UIO.openFile(dataFilename, dataFile, O_WRITE | O_CREAT | O_APPEND))
  {
    error(cr.last_error);
    return 1;
  }
  size = dataFile.fileSize();

  if (append(dataFile, frame.buffer, frame.length))
  {
    error(cr.last_error);
    return 1;
  }
  dataFile.close();

  // (3) Append to queue
  if (UIO.openFile(tmpFilename, tmpFile, O_RDWR | O_CREAT))
  {
    error(cr.last_error);
    return 2;
  }

  *(uint32_t *)(item + 3) = size;
  item[7] = (uint8_t) frame.length;
  if (append(tmpFile, item, 8))
  {
    error(cr.last_error);
    return 2;
  }

  // (4) Print frame to USB
  if (flags & FLAG_LOG_USB)
  {
    USB.ON();
    USB.flush();
    showBinaryFrame();
    USB.OFF();
  }

  return 0;
}


/******************************************************************************/
/* Function to communicate through OTA with remote unit
*
* Parameters: int OTA_duration  - duration in minute of the time window for OTA access to the unit
*/
void WaspUIO::OTA_communication(int OTA_duration)
{
  unsigned long start;
  unsigned long duration_ms;

  // OTA_duration is given in minutes
  duration_ms = OTA_duration * 60 * 1000;

  start = millis();
  do
  {
    if( xbeeDM.available() )
    {
      xbeeDM.treatData();
      // Keep inside this loop while a new program is being received
      while( xbeeDM.programming_ON  && !xbeeDM.checkOtapTimeout() )
      {
        if( xbeeDM.available() )
        {
          xbeeDM.treatData();
        }
      }
    }
  } while (! cr.timeout(start, duration_ms));
}


/**
 * Retturn true if the given sensor is to be read now.
 */
bool WaspUIO::action(uint8_t n, ...)
{
  va_list args;
  int idx;
  int value;
  bool yes = false;

  va_start(args, n);
  for (; n; n--)
  {
    idx = va_arg(args, int);
    assert(idx < RUN_LEN); // TODO Define __assert
    value = (int) actions[idx];
    if (value > 0)
    {
      if (minute % (cooldown * value) == 0)
      {
        yes = true;
        break;
      }
    }
  }
  va_end(args);

  return yes;
}


/******************************************************************************
 * PRIVATE FUNCTIONS                                                          *
 ******************************************************************************/

/**
 * Read a line from the given open file, not including the end-of-line
 * character. Store the read line in SD.buffer.
 *
 * Return the length of the line. Or -1 for EOF. Or -2 if error.
 */
void WaspUIO::getDataFilename(char* filename, uint8_t year, uint8_t month, uint8_t date)
{
  sprintf(filename, "%s/%02u%02u%02u.TXT", archive_dir, year, month, date);
}

/// Preinstantiate Objects /////////////////////////////////////////////////////
WaspUIO UIO = WaspUIO();


/******************************************************************************/
/* Function to read Mac Address
*/
const char* WaspUIO::readOwnMAC()
{
  bool on;

  // On
  on = xbeeDM.XBee_ON;
  if (! on)
  {
    xbeeDM.ON();
    delay(100);
  }

  // Convert mac address from array to string
  xbeeDM.getOwnMac();
  Utils.hex2str(xbeeDM.sourceMacHigh, myMac, 4);
  Utils.hex2str(xbeeDM.sourceMacLow, myMac + 8, 4);

  // Off
  if (! on)
  {
    xbeeDM.OFF();
  }

  return myMac;
}


/******************************************************************************/
/* Function to read last Packet hop signal strength absorption
*/

uint8_t WaspUIO::readRSSI2Frame(void)
{
  char sourceMAC[17];
  uint8_t sourcePower;
  int rssi;
  uint8_t error;

  xbeeDM.getRSSI();
  //sprintf(archiveFile,"%02u%02u%02u.TXT",RTC.year, RTC.month, RTC.date);
  Utils.hex2str(xbeeDM._srcMAC, sourceMAC, 8);
  sourcePower = xbeeDM.powerLevel;

  // check AT flag
  if( xbeeDM.error_AT == 0 )
  {
    //get rssi from getRSSI function and make conversion
    rssi = xbeeDM.valueRSSI[0];
    rssi *= -1;

    //trace(F("readRSSI2Frame: getRSSI(dBm): %d"), rssi);
  }

  // Create ASCII frame
  createFrame();
  ADD_SENSOR(SENSOR_MAC, (char*) myMac); // Add 2 unsigned longs
  ADD_SENSOR(SENSOR_RSSI, (int) rssi);
  ADD_SENSOR(SENSOR_MAC, sourceMAC);
  ADD_SENSOR(SENSOR_TX_PWR, (uint8_t) sourcePower);
  createFrame();
  // TODO RSSI should be stored in the health frame, no?

  return rssi;
}


/**
 * getNextAlarm
 *
 * Return the string of the next alarm.
 *
 */
void WaspUIO::nextAlarm()
{
  int value;
  int next = INT_MAX;

  for (uint8_t i=0; i < RUN_LEN; i++)
  {
    value = (int) actions[i];
    if (value > 0)
    {
      value *= cooldown;
      value = (minute / value) * value + value;
      if (value < next)
      {
        next = value;
      }
    }
  }

  if (next >= (24 * 60))
    next = 0;

  next_minute = next;
}

const char* WaspUIO::nextAlarm(char* alarmTime)
{
  uint32_t epoch = getEpochTime();
  minute = (epoch / 60) % (60 * 24); // minute of the day
  nextAlarm();

  // Format relative time to string, to be passed to deepSleep
  sprintf(alarmTime, "00:%02d:%02d:00", next_minute / 60, next_minute % 60);
  return alarmTime;
}

void WaspUIO::deepSleep()
{
  // Clear interruption flag & pin
  intFlag = 0;
  PWR.clearInterruptionPin();

  // Get next alarm time
  char alarmTime[12]; // "00:00:00:00"
  nextAlarm(alarmTime);

  // Reset watchdog
  int left = (next_minute == 0)? 1440 - minute: next_minute - minute;
  if (left < 59) // XXX Maximum is 59
  {
    RTC.setWatchdog(left + 1);
  }

  // Enable RTC interruption and sleep
  PWR.deepSleep(alarmTime, RTC_ABSOLUTE, RTC_ALM1_MODE3, ALL_OFF);

  // Awake: Reset if stuck for 4 minutes
  RTC.setWatchdog(UIO.loop_timeout);
}


/**
 * To reset v12 after a timeout. Requires our waspmoteapi fork.
 */
void onHAIwakeUP_after(void)
{
  if (_boot_version < 'H')
  {
    if (intFlag & RTC_INT)
    {
      RTC.ON();
      if (RTC.alarmTriggered & 2) // Alarm 2
      {
        intFlag &= ~(RTC_INT); RTC.alarmTriggered = 0;
        RTC.disableAlarm2();
        PWR.reboot();
      }
      RTC.OFF();
    }
  }
}
