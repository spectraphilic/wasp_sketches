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

  batteryType = Utils.readEEPROM(EEPROM_UIO_BATTERY_TYPE);
  if (batteryType != 1 && batteryType != 2)
  {
    batteryType = 1;
  }

  // Network
  uint8_t panid_low = Utils.readEEPROM(EEPROM_UIO_NETWORK+1);
  if (panid_low > NETWORK_PI_CS)
  {
    panid_low = NETWORK_BROADCAST; // Default
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
  onRegister = 0;

  loadTime(true); // Read temperature as well
  uint32_t epoch = getEpochTime();
  // Split the epoch time in 2: days since the epoch, and minute of the day
  day = epoch / (24 * 60 * 60);
  minute = (epoch / 60) % (60 * 24); // mins since the epoch modulus mins in a day

  readBattery();
}


/**
 * Functions to start and stop SD. Open and closes required files.
 */

void WaspUIO::startSD()
{
  // Already ON, or not SD.
  if (SPI.isSD || !hasSD)
  {
    return;
  }

  // On
  on(UIO_SD);

  // Create directories
  switch (SD.isDir(archive_dir))
  {
    case 1:
      break;
    case 0:
      SD.del(archive_dir);
    case -1:
      SD.mkdir((char*)archive_dir);
  }

  // Open log file (TODO Log rotate)
  if (flags & FLAG_LOG_SD)
  {
    if (!SD.openFile((char*)logFilename, &logFile, O_CREAT | O_WRITE | O_APPEND | O_SYNC))
    {
      cr.println(F("openFiles: opening log file failed"));
    }
  }

  // Open tmp file
  if (!SD.openFile((char*)tmpFilename, &tmpFile, O_CREAT | O_RDWR | O_APPEND | O_SYNC))
  {
    error(F("openFiles: opening tmp file failed"));
  }
}

void WaspUIO::stopSD()
{
  if (! SPI.isSD)
  {
    return;
  }

  // Close files
  if (tmpFile.isOpen())
  {
    tmpFile.close();
  }

  if (flags & FLAG_LOG_SD)
  {
    if (logFile.isOpen())
    {
      logFile.close();
    }
  }

  // Off
  SD.OFF();
}

/**
 * Function to log waspmote activity, adds message to a frame etc...
 *
 * Parameters: message
 *
 * Returns: bool             - true on success, false for error
 *
 * Current implementation uses an static variable. This means that (1) the
 * message length is limited (it will be truncated if too long), and (2) the
 * functions is not reentrant (cannot call itself). As a benefit it allows for
 * a predictable memory usage.
 */

void vlog(loglevel_t level, const char* message, va_list args)
{
  char buffer[128];
  size_t len;
  uint32_t seconds;
  uint16_t ms;

  // (1) Prepare the string to be printed
  memset(buffer, 0x00, sizeof(buffer));

  // Timestamp
  seconds = UIO.getEpochTime(ms);
  sprintf(buffer, "%lu.%03u ", seconds, ms);
  // Level
  len = strlen(buffer);
  sprintf(buffer + len, "%s ", cr.loglevel2str(level));

  // Message
  len = strlen(buffer);
  vsnprintf(buffer + len, sizeof(buffer) - len - 1, message, args);
  // Newline
  len = strlen(buffer);
  buffer[len] = '\n';

  // (2) Print to USB
  if (UIO.flags & FLAG_LOG_USB)
  {
    USB.ON();
    USB.flush(); // XXX This fixes a weird bug with XBee
    USB.print(buffer);
    USB.OFF();
  }

  // (3) Print to log file
  if (UIO.hasSD && (UIO.flags & FLAG_LOG_SD)) //&& UIO.logFile.isOpen())
  {
    UIO.startSD();

    // Print to log file
    if (UIO.logFile.write(buffer) == -1)
    {
      cr.println(F("ERROR vlog: failed writing to SD"));
    }
  }
}

void beforeSleep()
{
  UIO.stopSD();
}

void afterSleep()
{
#if USE_AGR
  if (UIO.isOn(UIO_PRESSURE))
  {
    SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_PRESSURE);
  }
  if (UIO.isOn(UIO_LEAFWETNESS))
  {
    SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_LEAF_WETNESS);
  }
  if (UIO.isOn(UIO_SENSIRION))
  {
    SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_SENSIRION);
  }
#endif

#if USE_SDI
  if (UIO.isOn(UIO_SDI12))
  {
    mySDI12.forceHold(); // XXX
  }
#endif

#if USE_I2C
  if (UIO.isOn(UIO_I2C))
  {
    // XXX
  }
  if (UIO.isOn(UIO_1WIRE))
  {
    // XXX
  }
#endif

  if (UIO.isOn(UIO_SD))
  {
    UIO.startSD();
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
  int32_t size;
  uint8_t item[8];
  char dataFilename[18]; // /data/YYMMDD.txt

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
  if (createFile(dataFilename))
  {
    error(F("frame2Sd: Failed to create %s"), dataFilename);
    return 1;
  }

  size = SD.getFileSize(dataFilename);
  if (size < 0)
  {
    error(F("frame2Sd: Failed to get size from %s"), dataFilename);
    return 1;
  }

  if (SD.append(dataFilename, frame.buffer, frame.length) == 0)
  {
    error(F("frame2Sd: Failed to append frame to %s"), dataFilename);
    return 1;
  }

  // (3) Append to queue
  *(uint32_t *)(item + 3) = (uint32_t) size;
  item[7] = (uint8_t) frame.length;
  if (append(tmpFile, item, 8) == -1)
  {
    error(F("frame2Sd: Failed to append to %s"), tmpFilename);
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

/**
 * Print the binary frame to USB.
 */

void WaspUIO::showBinaryFrame()
{
   uint8_t *p;
   uint8_t nbytes;
   char buffer[17];
   uint8_t i, j;
   char c;

   // Binary Frame
   cr.println(F("=== Binary Frame: %d fields in %d bytes ==="), frame.numFields, frame.length);
   p = frame.buffer;

   // Start delimiter
   if (strncmp((const char*) p, "<=>", 3) != 0)
   {
     cr.println(F("Error reading Start delimiter <=>"));
     return;
   }
   p += 3;

   // Frame type (TODO Print text identifier: Information, TimeOut, ...)
   // Don't clear the most significant bit, we already know it's zero
   cr.println(F("Frame type: %d"), *p++);

   // Number of bytes
   nbytes = *p++;
   //cr.println(F("Number of bytes left: %d"), nbytes);

   // Serial ID
   //cr.println(F("BOOT VERSION %c"), _boot_version);
   if (_boot_version >= 'G')
   {
     Utils.hex2str(p, buffer, 8);
     p += 8;
   }
   else
   {
     Utils.hex2str(p, buffer, 4);
     p += 4;
   }
   cr.println(F("Serial ID: 0x%s"), buffer);

   // Waspmote ID
   for (i = 0; i < 17 ; i++)
   {
     c = (char) *p++;
     if (c == '#')
     {
       buffer[i] = '\0';
       break;
     }
     buffer[i] = c;
   }
   if (c != '#')
   {
     cr.println(F("Error reading Waspmote ID"));
     return;
   }
   cr.println(F("Waspmote ID: %s"), buffer);

   // Sequence
   cr.println(F("Sequence: %d"), *p++);

   // Payload
   uint8_t sensor_id, nfields, type, decimals;
   uint8_t len;
   char name[20];
   char value[50];
   for (i = 0; i < frame.numFields; i++)
   {
     sensor_id = *p++;
     if (_boot_version >= 'G')
     {
       // v12
       strcpy_P(name, (char*)pgm_read_word(&(FRAME_SENSOR_TABLE[sensor_id])));
       nfields = (uint8_t)pgm_read_word(&(FRAME_SENSOR_FIELD_TABLE[sensor_id]));
       type = (uint8_t)pgm_read_word(&(FRAME_SENSOR_TYPE_TABLE[sensor_id]));
       decimals = (uint8_t)pgm_read_word(&(FRAME_DECIMAL_TABLE[sensor_id]));
     }
     else
     {
       // v15
       strcpy_P(name, (char*)pgm_read_word(&(SENSOR_TABLE[sensor_id])));
       nfields = (uint8_t)pgm_read_word(&(SENSOR_FIELD_TABLE[sensor_id]));
       type = (uint8_t)pgm_read_word(&(SENSOR_TYPE_TABLE[sensor_id]));
       decimals = (uint8_t)pgm_read_word(&(DECIMAL_TABLE[sensor_id]));
     }
     for (j = 0; j < nfields; j++)
     {
       if (type == 0) // uint8_t
       {
         cr.println(F("Sensor %d (%s): %d"), sensor_id, name, *p++);
       }
       else if (type == 1) // int
       {
         cr.println(F("Sensor %d (%s): %d"), sensor_id, name, *(int *)p);
         p += 2;
       }
       else if (type == 2) // double
       {
         Utils.float2String(*(float *)p, value, decimals);
         cr.println(F("Sensor %d (%s): %s"), sensor_id, name, value);
         p += 4;
       }
       else if (type == 3) // char*
       {
         len = *p++;
         if (len > sizeof(value) - 1)
         {
           cr.println(F("Error reading sensor value, string too long %d"), len);
           return;
         }
         strncpy(value, (char*) p, len);
         p += len;
         cr.println(F("Sensor %d (%s): %s"), sensor_id, name, value);
       }
       else if (type == 4) // uint32_t
       {
         cr.println(F("Sensor %d (%s): %lu"), sensor_id, name, *(uint32_t *)p);
         p += 4;
       }
       else if (type == 5) // uint8_t*
       {
         cr.println(F("Sensor %d (%s): unsupported type %d"), sensor_id, name, type); // TODO
       }
       else
       {
         cr.println(F("Sensor %d (%s): unexpected type %d"), sensor_id, name, type);
       }
     }
   }
   cr.println(F("=========================================="));
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

/*
 * On/Off devices
 */

void WaspUIO::on(uint8_t device)
{
  switch (device)
  {
    case UIO_SD:
      SD.ON();
      break;
#if USE_AGR
    case UIO_PRESSURE:
      SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_PRESSURE);
      break;
    case UIO_SENSIRION:
      SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_SENSIRION);
      break;
    case UIO_LEAFWETNESS:
      SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_LEAF_WETNESS);
      break;
#endif
#if USE_I2C
    case UIO_I2C:
      break;
    case UIO_1WIRE:
      break;
#endif
#if USE_SDI
    case UIO_SDI12:
      mySDI12.begin();
      break;
#endif
  }

  onRegister |= device;
}

void WaspUIO::off(uint8_t device)
{
  onRegister &= ~device;

  switch (device)
  {
    case UIO_SD:
      SD.OFF();
      break;
#if USE_AGR
    case UIO_PRESSURE:
      SensorAgrv20.setSensorMode(SENS_OFF, SENS_AGR_PRESSURE);
      break;
    case UIO_SENSIRION:
      SensorAgrv20.setSensorMode(SENS_OFF, SENS_AGR_SENSIRION);
      break;
    case UIO_LEAFWETNESS:
      SensorAgrv20.setSensorMode(SENS_OFF, SENS_AGR_LEAF_WETNESS);
      break;
#endif
#if USE_I2C
    case UIO_I2C:
      break;
    case UIO_1WIRE:
      break;
#endif
#if USE_SDI
    case UIO_SDI12:
      mySDI12.end();
      break;
#endif
  }
}

bool WaspUIO::isOn(uint8_t device)
{
  return onRegister & device;
}

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


void WaspUIO::readBattery()
{
  // Lead acid battery does not support reading voltage, yet
  if (UIO.batteryType == 2)
  {
    batteryLevel = 100;
  }
  else
  {
    batteryLevel = PWR.getBatteryLevel();
  }

  // Calculate the cooldown factor, depends on battery level
  cooldown = 1;
  // Power logic for lithium battery
  if (UIO.batteryType == 1)
  {
    if      (batteryLevel <= 30) { cooldown = 3; }
    else if (batteryLevel <= 40) { cooldown = 2; }
  }
  // TODO Logic for Lead acid battery
  else if (UIO.batteryType == 2)
  {
  }
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
