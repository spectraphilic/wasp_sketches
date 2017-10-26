/******************************************************************************
 * Includes
 ******************************************************************************/

#ifndef __WPROGRAM_H__
#include <WaspClasses.h>
#endif

#include "WaspUIO.h"


/******************************************************************************
* Constructors
******************************************************************************/

SDI12 mySDI12(6); // DATAPIN = 6


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

  wakeup_period = Utils.readEEPROM(EEPROM_UIO_WAKEUP_PERIOD);
  batteryType = Utils.readEEPROM(EEPROM_UIO_BATTERY_TYPE);
  if (batteryType != 1 && batteryType != 2)
  {
    batteryType = 1;
  }

  // Network
  uint8_t panid_low = Utils.readEEPROM(EEPROM_UIO_NETWORK+1);
  if (panid_low > NETWORK_RASP)
  {
    panid_low = NETWORK_BROADCAST; // Default
  }
  memcpy_P(&network, &networks[panid_low], sizeof network);

  // Sensors
#if USE_AGR
  sensor_sensirion = Utils.readEEPROM(EEPROM_SENSOR_SENSIRION);
  sensor_pressure = Utils.readEEPROM(EEPROM_SENSOR_PRESSURE);
  sensor_leafwetness = Utils.readEEPROM(EEPROM_SENSOR_LEAFWETNESS);
#endif
#if USE_SDI
  sensor_ctd10 = Utils.readEEPROM(EEPROM_SENSOR_CTD10);
  sensor_ds2 = Utils.readEEPROM(EEPROM_SENSOR_DS2);
#endif
#if USE_I2C
  sensor_ds1820 = Utils.readEEPROM(EEPROM_SENSOR_DS1820);
  sensor_bme280 = Utils.readEEPROM(EEPROM_SENSOR_BME280);
#endif

  // Log level
  cr.loglevel = (loglevel_t) Utils.readEEPROM(EEPROM_UIO_LOG_LEVEL);

  /*** 2. Autodetect hardware ***/
  cr.print(F("Hardware autodetect in progress"));
  hasSD = SD.ON();
  SD.OFF();
  hasGPS = GPS.ON();
  GPS.OFF();
  cr.print(F("Hardware autodetect done (SD=%d GPS=%d)"), hasSD, hasGPS);

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
   initTime();
   readBattery();
   onRegister = 0;
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
      cr.print(F("openFiles: opening log file failed"));
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
      cr.print(F("ERROR vlog: failed writing to SD"));
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

#if FRAME_BINARY
  frame.createFrame(BINARY);
#else
  frame.createFrame(ASCII);
#endif

  // In binary frames, the timestamp must be first, that's what I deduce from
  // frame.addTimestamp
  frame.addSensor(SENSOR_TST, epochTime);
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

#if FRAME_BINARY
  if (SD.append(dataFilename, frame.buffer, frame.length) == 0)
#else
  if (SD.appendln(dataFilename, frame.buffer, frame.length) == 0)
#endif
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
    showFrame();
    USB.OFF();
  }

  return 0;
}

/**
 * Print the frame to USB. If it's ASCII just call frame.showFrame; if it's
 * binary decode and print it.
 */

void WaspUIO::showFrame()
{
   uint8_t *p;
   uint8_t nbytes;
   char waspmote_id[17];
   uint8_t i, j;
   char c;

   // ASCII Frame
   if (frame.buffer[3] & 128)
   {
     frame.showFrame();
     return;
   }

   //frame.showFrame();

   // Binary Frame
   cr.print(F("=== Binary Frame: %d fields in %d bytes ==="), frame.numFields, frame.length);
   p = frame.buffer;

   // Start delimiter
   if (strncmp((const char*) p, "<=>", 3) != 0)
   {
     cr.print(F("Error reading Start delimiter <=>"));
     return;
   }
   p += 3;

   // Frame type (TODO Print text identifier: Information, TimeOut, ...)
   // Don't clear the most significant bit, we already know it's zero
   cr.print(F("Frame type: %d"), *p++);

   // Number of bytes
   nbytes = *p++;
   //cr.print(F("Number of bytes left: %d"), nbytes);

   // Serial ID
   //cr.print(F("BOOT VERSION %c"), _boot_version);
   if (_boot_version >= 'G')
   {
     cr.print(F("Serial ID (v15): 0x%02X%02X%02X%02X%02X%02X%02X%02X"),
              *p, *(p+1), *(p+2), *(p+3), *(p+4), *(p+5), *(p+6), *(p+7));
     p += 8;
   }
   else
   {
     cr.print(F("Serial ID (v12): 0x%02X%02X%02X%02X"),
              *p, *(p+1), *(p+2), *(p+3));
     p += 4;
   }

   // Waspmote ID
   //memset(waspmote_id, 0x00, sizeof(waspmote_id));
   for (i = 0; i < 17 ; i++)
   {
     c = (char) *p++;
     if (c == '#')
     {
       waspmote_id[i] = '\0';
       break;
     }
     waspmote_id[i] = c;
   }
   if (c != '#')
   {
     cr.print(F("Error reading Waspmote ID"));
     return;
   }
   cr.print(F("Waspmote ID: %s"), waspmote_id);

   // Sequence
   cr.print(F("Sequence: %d"), *p++);

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
         cr.print(F("Sensor %d (%s): %d"), sensor_id, name, *p++);
       }
       else if (type == 1) // int
       {
         cr.print(F("Sensor %d (%s): %d"), sensor_id, name, *(int *)p);
         p += 2;
       }
       else if (type == 2) // double
       {
         Utils.float2String(*(float *)p, value, decimals);
         cr.print(F("Sensor %d (%s): %s"), sensor_id, name, value);
         p += 4;
       }
       else if (type == 3) // char*
       {
         len = *p++;
         if (len > sizeof(value) - 1)
         {
           cr.print(F("Error reading sensor value, string too long %d"), len);
           return;
         }
         strncpy(value, (char*) p, len);
         p += len;
         cr.print(F("Sensor %d (%s): %s"), sensor_id, name, value);
       }
       else if (type == 4) // uint32_t
       {
         cr.print(F("Sensor %d (%s): %lu"), sensor_id, name, *(uint32_t *)p);
         p += 4;
       }
       else if (type == 5) // uint8_t*
       {
         cr.print(F("Sensor %d (%s): unsupported type %d"), sensor_id, name, type); // TODO
       }
       else
       {
         cr.print(F("Sensor %d (%s): unexpected type %d"), sensor_id, name, type);
       }
     }
   }
   cr.print(F("=========================================="));
}


uint8_t WaspUIO::setTime(uint8_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
  uint8_t error;
  uint8_t rtcON = RTC.isON;

  // Turn on RTC if required
  if (! rtcON)
  {
    RTC.ON();
  }

  // Set time
  uint8_t dow = RTC.dow(year, month, day);
  error = RTC.setTime(year, month, day, dow, hour, minute, second);
  if (error)
  {
    error(F("setTime: RTC.setTime failed (please double check the syntax)"));
  }

  // Turn off RTC only if it was off when this method was called
  if (! rtcON)
  {
    RTC.OFF();
  }

  return error;
}

/**
 * Function to receive, parse and update RTC from GPS_sync frame
 *
 * Parameters: void
 *
 * Returns: uint8_t (0 on success, 1 on error)
 */

uint8_t WaspUIO::receiveGPSsyncTime()
{
  char * pch; // Define pointer for string
  unsigned long epoch; // Define variable for epoch time
  timestamp_t time;

  debug(F("receiveGPSsyncTime: got frame %s"), xbeeDM._payload);

  pch = strstr((const char*)xbeeDM._payload, "TST:");
  //trace(F("receiveGPSsyncTime: pch = %s"), pch);
  epoch = strtoul(pch+4, NULL, 10);
  epoch = epoch + 2; // Add some seconds for the delays, FIXME Check this!!!
  // Break Epoch time into UTC time
  RTC.breakTimeAbsolute(epoch, &time);

  // Setting time [yy:mm:dd:dow:hh:mm:ss]
  // '0' on succes, '1' otherwise
  if (RTC.setTime(time.year, time.month, time.date, time.day, time.hour, time.minute, time.second) != 0)
  {
    error(F("No RTC update from GPS"));
    return 1;
  }

  // Success!!
  info(F("Got GPStime, RTC updated"));
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


/******************************************************************************/
/* Function to read the Maxbotix (MB7389) sensor
*
* Parameters: uint8_t samples  - Number of readings to sample
*                              - Defult is set to 5
*
* Returns: uint8_t             - median value of samples
*
* Wiring: GND -> GND, V+ -> 3V3, pin5 -> AUX SERIAL 1RX
*/
uint16_t WaspUIO::readMaxbotixSerial(uint8_t nsamples)
{
  const int bytes = 4; // Number of bytes to read
  char data_buffer[bytes]; // Store serial data
  int sample; // Store each sample
  uint16_t samples[nsamples];
  uint8_t i, j;

  Utils.setMuxAux1(); // check the manual to find out where you connect the sensor

  PWR.setSensorPower(SENS_3V3,SENS_ON); //  sensor needs 3.3 voltage
  delay(1000);

  beginSerial(9600,1); // set boud rate to 9600

  for (j = 0; j < nsamples;)
  {
    // flush and wait for a range reading
    serialFlush(1);

    while (!serialAvailable(1) || serialRead(1) != 'R');

    // read the range
    for (i = 0; i < bytes; i++)
    {
      while (!serialAvailable(1));

      data_buffer[i] = serialRead(1);
    }

    sample = atoi(data_buffer);

    if (sample<=300 || sample>=5000)
    {
      debug(F("readMaxbotixSerial: NaN"));
      delay(10);
    }
    else
    {
      samples[j] = (uint16_t) sample;
      j++;
      debug(F("readMaxbotixSerial: sample = %d"), sample);
      // add a function for median value....  construct an array...
      delay(1000);
    }
  }

  // Bubble sort
  bool done = false;
  while (done == false)
  {
    done = true;
    for (j = 0; j < nsamples; j++)
    {
      // numbers are out of order - swap
      if (samples[j] > samples[j+1])
      {
        uint16_t temp = samples[j+1];
        samples [j+1] = samples[j];
        samples [j] = temp;
        done = false;
      }
    }
  }

  // Median
  if (nsamples % 2 == 1)
  {
    return samples[nsamples/2];
  }
  else
  {
    return (samples[nsamples/2] + samples[nsamples/2 - 1]) / 2;
  }
}

/**
 * Retturn true if the given sensor is to be read now.
 */
bool WaspUIO::action(uint8_t n, ...)
{
  va_list args;
  int value;
  bool yes = false;

  va_start(args, n);
  for (; n; n--)
  {
    value = va_arg(args, int);
    if (value > 0)
    {
      if ((time.hour * 60 + time.minute) % (period * value) == 0)
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
const char* WaspUIO::readOwnMAC(char* mac)
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
  Utils.hex2str(xbeeDM.sourceMacHigh, mac, 4);
  Utils.hex2str(xbeeDM.sourceMacLow, mac + 8, 4);

  // Off
  if (! on)
  {
    xbeeDM.OFF();
  }

  // Log
  debug(F("readOwnMAC: My MAC address is %s"), mac);

  return mac;
}


/******************************************************************************/
/* Function to read last Packet hop signal strength absorption
*/

uint8_t WaspUIO::readRSSI2Frame(void)
{
  char ownMAC[17];
  char sourceMAC[17];
  uint8_t sourcePower;
  int rssi;
  uint8_t error;

  // 1. read own MAC address
  readOwnMAC(ownMAC);

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
  ADD_SENSOR(SENSOR_MAC, (char*) ownMAC);
  ADD_SENSOR(SENSOR_RSSI, (int) rssi);
  ADD_SENSOR(SENSOR_MAC, sourceMAC);
  ADD_SENSOR(SENSOR_TX_PWR, (uint8_t) sourcePower);
  createFrame();
  // TODO RSSI should be stored in the health frame, no?

  return rssi;
}


/**
 * initTime
 *
 * Initialize the epochTime and start variables.
 * (and rtc_temp and batteryLevel, TODO Change function name)
 */

void WaspUIO::initTime()
{
  uint8_t rtcON = RTC.isON;

  // Turn on RTC if required
  if (! rtcON)
  {
    RTC.ON();
  }

  epochTime = RTC.getEpochTime();
  start = millis();
  cr.sleep_time = 0;

  // Read temperature now that RTC is ON (v12 only), it takes less than 2ms
  if (_boot_version < 'H')
  {
    rtc_temp = RTC.getTemperature();
  }

  // Turn off RTC only if it was off when this method was called
  if (! rtcON)
  {
    RTC.OFF();
  }

  // Update RTC time at least once. Keep minute and hour for later.
  RTC.breakTimeAbsolute(getEpochTime(), &time);
}


void WaspUIO::readBattery()
{


  // Battery
  batteryLevel = PWR.getBatteryLevel();

  // Power logic for lithium battery
  if (UIO.batteryType == 1)
  {
    // Find out sampling period, in minutes. The value must be a factor of 60*24
    // (number of minutes in a day).
    // Different values for different battery levels.
    if (batteryLevel <= 30)
    {
      period = wakeup_period * 3; // 15 minutes
    }
    else if (batteryLevel <= 40)
    {
      period = wakeup_period * 2; // 10 minutes
    }
    else
    {
      period = wakeup_period * 1; // 5 minutes
    }
  }

  // Logic for Lead acid battery
  else if (UIO.batteryType == 2)
  {
    period = wakeup_period * 1;
  }
}

/**
 * getEpochTime
 *
 * Return the current time (seconds since the epoch), from epochTime and start
 * (without calling the RTC).
 *
 * Optionally with milliseconds precision.
 */

unsigned long WaspUIO::getEpochTime()
{
  uint32_t ms = cr.millisDiff(start) + cr.sleep_time;
  return epochTime + ms / 1000;
}

unsigned long WaspUIO::getEpochTime(uint16_t &ms)
{
  uint32_t diff = cr.millisDiff(start) + cr.sleep_time;

  ms = diff % 1000;
  return epochTime + diff / 1000;
}


/**
 * getNextAlarm
 *
 * Return the string of the next alarm.
 *
 */
const char* WaspUIO::getNextAlarm(char* alarmTime)
{
  // TODO Be more clever, check the enabled sensors (and network), and only
  // wake up when something needs to be done

  // Format relative time to string, to be passed to deepSleep
  RTC.breakTimeAbsolute(getEpochTime(), &time);
  uint8_t alarmMinute = (time.minute / period) * period + period;
  if (alarmMinute >= 60)
    alarmMinute = 0;
  sprintf(alarmTime, "00:00:%02d:00", alarmMinute);

  return alarmTime;
}

void WaspUIO::deepSleep()
{
  // Clear interruption flag & pin
  intFlag = 0;
  PWR.clearInterruptionPin();

  // Reset watchdog
  RTC.setWatchdog(period + 1);

  // Enable RTC interruption and sleep
  char alarmTime[12]; // "00:00:00:00"
  getNextAlarm(alarmTime);
  PWR.deepSleep(alarmTime, RTC_ABSOLUTE, RTC_ALM1_MODE4, ALL_OFF);
  RTC.setWatchdog(UIO.loop_timeout); // Reset if stuck for 4 minutes
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
