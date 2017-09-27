/******************************************************************************
 * Includes
 ******************************************************************************/

#ifndef __WPROGRAM_H__
#include <WaspClasses.h>
#endif

#include "WaspUIO.h"
#include <WaspSD.h>
#include <WaspXBeeDM.h>
#include <WaspFrame.h>
#include <WaspSensorAgr_v20.h>
#include <WaspGPS.h>
#include <SDI12.h>
#include <BME280.h>


SDI12 mySDI12(6); // DATAPIN = 6


/******************************************************************************
* Constructors
******************************************************************************/


/******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * createFile
 *
 * Creates the file with the given filename in the SD card. Returns 0 on
 * success, 1 on error.
 *
 * If the file already exists it is not considered an error.
*/

uint8_t WaspUIO::createFile(const char* filename)
{
  int8_t isFile = SD.isFile(filename);

  // Already created
  if (isFile == 1)
  {
    //trace(F("Failed to create %s, it already exists"), filename);
    return 0;
  }

  // Create
  if (isFile == -1)
  {
    if (SD.create(filename))
    {
      debug(F("%s created"), filename);
      return 0;
    }

    error(F("Failed to create %s, error %d"), filename, SD.flag);
    return 1;
  }

  //if (isFile == 0)
  error(F("Exists but not a file %s"), filename);
  return 1;
}

/**
 * Function to be called first in setup()
 */
void WaspUIO::onSetup()
{
  /*** 1. Read configuration from EEPROM ***/

  // Flags
  flags = Utils.readEEPROM(EEPROM_UIO_FLAGS);
  featureUSB = flags & FLAG_LOG_USB;
  featureNetwork = flags & FLAG_NETWORK;

  wakeup_period = Utils.readEEPROM(EEPROM_UIO_WAKEUP_PERIOD);
  batteryType = Utils.readEEPROM(EEPROM_UIO_BATTERY_TYPE);
  if (batteryType != 1 && batteryType != 2)
  {
    batteryType = 1;
  }

  // Network
  uint8_t panid_low = Utils.readEEPROM(EEPROM_UIO_NETWORK+1);
  if (panid_low > NETWORK_OTHER)
  {
    panid_low = 4; // Default: Other
                   // Be careful to update this if the networks table changes
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

  /*** 2. Autodetect hardware ** */
  SD.ON();
  hasSD = (SD.isSD() == 1);
  SD.OFF();

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
}


/******************************************************************************/
/* Function to intialize the waspmote and its XBee to a digimesh network setting
*     Parameter:    - network, 'Finse' .... add other network setting if needed
*
*   =========== WARNING: MAC address must be with capital letters =========
*/

void WaspUIO::setNetwork(network_t value)
{
  // Check input parameter is valid
  if (value < NETWORK_FINSE || NETWORK_OTHER < value)
  {
      cr.print(F("ERROR No network configuration"));
      return;
  }

  // Enable network
  UIO.flags |= FLAG_NETWORK;
  updateEEPROM(EEPROM_UIO_FLAGS, UIO.flags);

  // Save panID to EEPROM
  memcpy_P(&network, &networks[value], sizeof network);
  if (
    updateEEPROM(EEPROM_UIO_NETWORK, network.panid[0]) &&
    updateEEPROM(EEPROM_UIO_NETWORK+1, network.panid[1])
  )
  {
    cr.print(F("INFO Network id saved to EEPROM"));
  }
  else
  {
    cr.print(F("ERROR Saving network id to EEPROM failed"));
  }

  initNet();
}

void WaspUIO::initNet()
{
  uint8_t addressing = UNICAST_64B;
  network_t value = (network_t) network.panid[1]; // panid low byte

  // Check input parameter is valid
  if (value < NETWORK_FINSE || NETWORK_OTHER < value)
  {
      cr.print(F("ERROR No network configuration"));
      return;
  }

  // First enable network
  featureNetwork = 1;

  // Addressing
  memcpy_P(&network, &networks[value], sizeof network);
  cr.print(F("INFO Configure network: %s"), network.name);
  if (strcmp(network.rx_address, "000000000000FFFF") == 0)
  {
    addressing = BROADCAST_MODE;
  }

  // This is common to all networks, for now
  frame.setID((char*) "");
  //Utils.setAuthKey(key_access);

  // Set Frame size. Will be 73 bytes for XBeeDM-pro S1
  // linkEncryption = DISABLED (not supported by DIGIMESH, apparently)
  // AESEncryption = DISABLED
  frame.setFrameSize(DIGIMESH, addressing, DISABLED, DISABLED);
  cr.print(F("DEBUG Frame size is %d"), frame.getFrameSize());

  // init XBee
  xbeeDM.ON();
  delay(50);

  // XXX Reduce the number of retries to reduce the time it is lost in send
  // failures (default is 3).
  // 3 retries ~ 5s ; 2 retries ~ 3.5 s ; 1 retry ~ 2.4s
  xbeeDM.setSendingRetries(2);

  // Set channel, check AT commmand execution flag
  xbeeDM.setChannel(network.channel);
  if( xbeeDM.error_AT == 0 ) {
    cr.print(F("DEBUG Channel set OK")); // Implement string in print function
  } else {
    cr.print(F("ERORR with setChannel()"));
  }

  // set PANID, check AT commmand execution flag
  xbeeDM.setPAN(network.panid);
  if( xbeeDM.error_AT == 0 ) {
    cr.print(F("DEBUG PAN ID set OK"));
  } else {
    cr.print(F("ERROR calling 'setPAN()'"));
  }

  // set encryption mode (1:enable; 0:disable), check AT commmand execution flag
  // XXX Should we use encryption
  xbeeDM.setEncryptionMode(0);
  if( xbeeDM.error_AT == 0 ) {
    cr.print(F("DEBUG AES encryption Mode OK"));
  } else {
    cr.print(F("ERROR no setEncryptionMode()"));
  }

  // set encryption key, check AT commmand execution flag
  xbeeDM.setLinkKey(encryptionKey);
  if( xbeeDM.error_AT == 0 ) {
    cr.print(F("DEBUG AES encryption key set OK"));
  } else {
    cr.print(F("ERROR with setLinkKey()"));
  }

  // write values to XBee module memory, check AT commmand execution flag
  xbeeDM.writeValues();
  if( xbeeDM.error_AT == 0 ) {
    cr.print(F("DEBUG Changes stored OK"));
  } else {
    cr.print(F("ERROR with writeValues()"));
  }
  xbeeDM.OFF();
}

/**
 * Functions to start and stop SD. Open and closes required files.
 */

void WaspUIO::startSD()
{
  if (! hasSD)
  {
    return;
  }

  SD.ON();

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
      error(F("openFiles: opening log file failed"));
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
  if (UIO.featureUSB)
  {
    USB.ON();
    USB.flush(); // XXX This fixes a weird bug with XBee
    USB.print(buffer);
    USB.OFF();
  }

  // (3) Print to log file
  if (UIO.hasSD && (UIO.flags & FLAG_LOG_SD) && UIO.logFile.isOpen())
  {
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
  UIO.startSD();
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
    warn(F("frame2Sd: SD card not available"));
    return 1;
  }

  // (1) Get the date
  item[0] = RTC.year;
  item[1] = RTC.month;
  item[2] = RTC.date;
  getDataFilename(dataFilename, item[0], item[1], item[2]);

  // (2) Store frame in archive file
  if (UIO.createFile(dataFilename))
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
  if (featureUSB)
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
     // v15
     p += 8;
     cr.print(F("Serial ID: TODO"));
   }
   else
   {
     // v12
     cr.print(F("Serial ID: %lu"), *(uint32_t *)p);
     p += 4; // v12
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
 * Ask the user for input through the USB cable
 *
 * Parameters:
 * - prompt    String printed to USB.
 * - timeout   Number of seconds to wait for input, or 0 to wait forever (max 255).
 *
 * Returns:    String read from the USB cable, or NULL if timeout.
 */

const char* WaspUIO::input(char* buffer, size_t size, const __FlashStringHelper * prompt, unsigned long timeout)
{
  int i = 0;

  cr.print(prompt);
  USB.flush();

  // Wait for available data, or timeout
  if (timeout > 0)
  {
    unsigned long timeStart = millis();
    while (USB.available() == 0)
    {
      if (cr.millisDiff(timeStart) > timeout)
      {
        return NULL;
      }
    }
  }
  else
  {
    while (USB.available() == 0);
  }

  // Read the data
  for (i=0; i < size - 1; i++)
  {
    // Could be optimized to read as many chars as USB.available says in one
    // go. But this is a cold path, do don't bother.
    if (USB.available() == 0)
      break;

    buffer[i] = (char) USB.read();
    if ((buffer[i] == '\r') || (buffer[i] == '\n'))
      break;
  }

  buffer[i] = '\0';
  return buffer;
}

/**
 * Function to enter interactive mode
 *
 * Parameters: void
 * Returns   : void
 *
 */

const char* WaspUIO::menuFormatBattery(char* dst, size_t size)
{
  dst[0] = '\0';
  if      (batteryType == 1) strncpy_F(dst, F("Lithium-ion"), size);
  else if (batteryType == 2) strncpy_F(dst, F("Lead acid"), size);
  return dst;
}

const char* WaspUIO::menuFormatLog(char* dst, size_t size)
{
  dst[0] = '\0';
  if (flags & FLAG_LOG_USB) strnjoin_F(dst, size, F(", "), F("USB"));
  if (flags & FLAG_LOG_SD)  strnjoin_F(dst, size, F(", "), F("SD"));
  return dst;
}

const char* WaspUIO::menuFormatNetwork(char* dst, size_t size)
{
  if (featureNetwork == 0)
  {
    strncpy_F(dst, F("Disabled"), size);
  }
  else
  {
    strncpy(dst, network.name, size);
  }

  return dst;
}

const char* WaspUIO::menuFormatSensors(char* dst, size_t size)
{
  dst[0] = '\0';
#ifdef USE_AGR
  if (sensor_sensirion)   strnjoin_F(dst, size, F(", "), F("Sensirion (%d)"), sensor_sensirion);
  if (sensor_pressure)    strnjoin_F(dst, size, F(", "), F("Pressure (%d)"), sensor_pressure);
  if (sensor_leafwetness) strnjoin_F(dst, size, F(", "), F("Leaf Wetness (%d)"), sensor_leafwetness);
#endif
#ifdef USE_SDI
  if (sensor_ctd10)       strnjoin_F(dst, size, F(", "), F("CTD-10 (%d)"), sensor_ctd10);
  if (sensor_ds2)         strnjoin_F(dst, size, F(", "), F("DS-2 (%d)"), sensor_ds2);
#endif
#ifdef USE_I2C
  if (sensor_ds1820)      strnjoin_F(dst, size, F(", "), F("DS1820 (%d)"), sensor_ds1820);
  if (sensor_bme280)      strnjoin_F(dst, size, F(", "), F("BME-280 (%d)"), sensor_bme280);
#endif
  if (! dst[0])           strncpy_F(dst, F("(none)"), size);
  return dst;
}


void WaspUIO::menu()
{
  char c;
  char buffer[150];
  size_t size = sizeof(buffer);

  RTC.ON();

  // Go interactive or not
  if (input(buffer, sizeof(buffer), F("Press Enter to start interactive mode. Wait 2 seconds to skip:"), 2000) == NULL)
  {
    goto exit;
  }

  do {
    // Menu
    cr.print();
    cr.print(F("1. Time    : %s"), RTC.getTime());
    cr.print(F("2. Log     : level=%s output=%s"), cr.loglevel2str(cr.loglevel), menuFormatLog(buffer, size));
    cr.print(F("3. Network : %s"), menuFormatNetwork(buffer, size));
    cr.print(F("4. Wakeup  : %d minutes"), wakeup_period);
    cr.print(F("5. Sensors : %s"), menuFormatSensors(buffer, size));
    cr.print(F("6. Battery : %s (%d %%)"), menuFormatBattery(buffer, size), batteryLevel);
    if (hasSD)
    {
      cr.print(F("8. SD"));
    }
    else
    {
      cr.print(F("SD card is missing!"));
    }

    cr.print(F("9. Exit"));
    cr.print();
    input(buffer, size, F("==> Enter numeric option:"), 0);
    c = buffer[0];
    if      (c == '1') { menuTime(); }
    else if (c == '2') { menuLog(); }
    else if (c == '3') { menuNetwork(); }
    else if (c == '4') { menuWakeup(); }
    else if (c == '5') { menuSensors(); }
    else if (c == '6') { menuBatteryType(); }
    else if (c == '8') { if (hasSD) menuSD(); }
    else if (c == '9') { goto exit; }
  } while (true);

exit:
  cr.print();
  RTC.OFF();
}

/**
 * Function to ask the user for the time and set RTC
 *
 * Parameters: void
 * Returns   : void
 *
 */

void WaspUIO::menuTime()
{
  char str[80];

  do
  {
    cr.print();
    cr.print(F("1. Set time manually"));
    cr.print(F("2. Set time from GPS"));
    cr.print(F("9. Exit"));
    cr.print();
    input(str, sizeof(str), F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '1':
        menuTimeManual();
        break;
      case '2':
        cr.print(F("Setting time from GPS, please wait, it may take a few minutes"));
        taskGps();
        break;
      case '9':
        cr.print();
        return;
    }
  } while (true);
}


void WaspUIO::menuTimeManual()
{
  unsigned short year, month, day, hour, minute, second;
  char str[80];

  do
  {
    input(str, sizeof(str), F("Set new time (format is yy:mm:dd:hh:mm:ss). Press Enter to leave it unchanged:"), 0);
    if (strlen(str) == 0)
      return;

    // Set new time
    if (sscanf(str, "%hu:%hu:%hu:%hu:%hu:%hu", &year, &month, &day, &hour, &minute, &second) == 6)
    {
      setTime(year, month, day, hour, minute, second);
      initTime();
      cr.print(F("Current time is %s"), RTC.getTime());
      return;
    }
  } while (true);
}


/**
 * Functions to manage Agr board sensors
 *
 * Parameters: void
 * Returns   : void
 *
 */

const char* WaspUIO::sensorStatus(uint8_t sensor)
{
  switch (sensor)
  {
    case 0:
      return "disabled";
    case 1:
      return "1";
    case 2:
      return "2";
    case 3:
      return "3";
    default:
      return "undefined";
  }
}

void WaspUIO::menuSensor(uint16_t sensor, uint8_t &value)
{
  char str[80];

  do
  {
    cr.print();
    cr.print(F("Current state is: %s"), sensorStatus(value));
    cr.print(F("0. Disable"));
    cr.print(F("1. One period (eg 5 minutes)"));
    cr.print(F("2. Two periods (eg 10 minutes)"));
    cr.print(F("3. Three periods (eg 15 minutes)"));
    if (sensor == EEPROM_SENSOR_CTD10 || sensor == EEPROM_SENSOR_DS2)
    {
      cr.print(F("8. Identification"));
    }
    cr.print(F("9. Exit"));
    cr.print();
    input(str, sizeof(str), F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '0':
        value = 0;
        goto update;
      case '1':
        value = 1;
        goto update;
      case '2':
        value = 2;
        goto update;
      case '3':
        value = 3;
        goto update;
      case 8:
        if (sensor == EEPROM_SENSOR_CTD10 || sensor == EEPROM_SENSOR_DS2)
        {
          cr.print(F("Enabling SDI-12"));
          PWR.setSensorPower(SENS_5V, SENS_ON);
          mySDI12.begin();

          if (sensor == EEPROM_SENSOR_CTD10)
          {
            mySDI12.identification(0);
          }
          else if (sensor == EEPROM_SENSOR_DS2)
          {
            mySDI12.identification(1);
          }

          cr.print(F("Disabling SDI-12"));
          mySDI12.end();
          PWR.setSensorPower(SENS_5V, SENS_OFF);
        }
        break;
      case '9':
        cr.print();
        return;
    }
  } while (true);

update:
  updateEEPROM(sensor, value);
  cr.print();
  return;
}

/**
 * Function to configure the network
 *
 * Parameters: void
 * Returns   : void
 *
 */

void WaspUIO::menuNetwork()
{
  char str[80];

  do
  {
    cr.print();
    cr.print(F("0. Disable"));
    cr.print(F("1. Finse"));
    cr.print(F("2. Gateway"));
    cr.print(F("3. Broadcast"));
    cr.print(F("4. Finse alt"));
    cr.print(F("5. Other"));
    cr.print(F("9. Exit"));
    cr.print();
    input(str, sizeof(str), F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '0': // Disable
        UIO.flags &= ~FLAG_NETWORK;
        updateEEPROM(EEPROM_UIO_FLAGS, UIO.flags);
        featureNetwork = 0;
        return;
      case '1':
        setNetwork(NETWORK_FINSE);
        return;
      case '2':
        setNetwork(NETWORK_GATEWAY);
        return;
      case '3':
        setNetwork(NETWORK_BROADCAST);
        return;
      case '4':
        setNetwork(NETWORK_FINSE_ALT);
        return;
      case '5':
        setNetwork(NETWORK_OTHER);
        return;
      case '9':
        cr.print();
        return;
    }
  } while (true);
}

/**
 * Functions to configure logging
 *
 * Parameters: void
 * Returns   : void
 *
 */

const char* WaspUIO::flagStatus(uint8_t flag)
{
  return (flags & flag)? "enabled": "disabled";
}

void WaspUIO::menuLog()
{
  char str[80];
  char* level;

  do
  {
    cr.print();
    cr.print(F("1. Log to SD (%s)"), flagStatus(FLAG_LOG_SD));
    cr.print(F("2. Log to USB (%s)"), flagStatus(FLAG_LOG_USB));
    cr.print(F("3. Choose the log level (%s)"), cr.loglevel2str(cr.loglevel));
    cr.print(F("9. Exit"));
    cr.print();
    input(str, sizeof(str), F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '1':
        menuLog2(FLAG_LOG_SD, "SD");
        break;
      case '2':
        menuLog2(FLAG_LOG_USB, "USB");
        break;
      case '3':
        menuLogLevel();
        break;
      case '9':
        cr.print();
        return;
    }
  } while (true);
}

void WaspUIO::menuLog2(uint8_t flag, const char* var)
{
  char str[80];

  do
  {
    cr.print(F("Type 1 to enable %s output, 0 to disable, Enter to leave:"), var);
    input(str, sizeof(str), F(""), 0);
    if (strlen(str) == 0)
      return;

    switch (str[0])
    {
      case '0':
        UIO.flags &= ~flag;
        updateEEPROM(EEPROM_UIO_FLAGS, UIO.flags);
        featureUSB = 0;
        return;
      case '1':
        UIO.flags |= flag;
        updateEEPROM(EEPROM_UIO_FLAGS, UIO.flags);
        featureUSB = 1;
        return;
    }
  } while (true);
}

void WaspUIO::menuBatteryType()
{
  char str[80];
  do{
    cr.print();
    cr.print(F("1. Lithium-ion"));
    cr.print(F("2. Lead acid"));
    cr.print(F("9. Exit"));
    cr.print();

    input(str, sizeof(str), F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '1':
        batteryType = 1;
        break;
      case '2':
        batteryType = 2;
        break;
      case '9':
        return;
      default:
        continue;
    }
    updateEEPROM(EEPROM_UIO_BATTERY_TYPE, batteryType);
    return;
  } while (true);
}

void WaspUIO::menuLogLevel()
{
  char str[80];

  do
  {
    cr.print();
    cr.print(F("0. Off"));
    cr.print(F("1. Fatal"));
    cr.print(F("2. Error"));
    cr.print(F("3. Warning"));
    cr.print(F("4. Info"));
    cr.print(F("5. Debug"));
    cr.print(F("6. Trace"));
    cr.print(F("9. Exit"));
    cr.print();
    input(str, sizeof(str), F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '0':
        cr.loglevel = OFF;
        break;
      case '1':
        cr.loglevel = FATAL;
        break;
      case '2':
        cr.loglevel = ERROR;
        break;
      case '3':
        cr.loglevel = WARN;
        break;
      case '4':
        cr.loglevel = INFO;
        break;
      case '5':
        cr.loglevel = DEBUG;
        break;
      case '6':
        cr.loglevel = TRACE;
        break;
      case '9':
        break;
    }
    updateEEPROM(EEPROM_UIO_LOG_LEVEL, (uint8_t) cr.loglevel);
    cr.print();
    return;
  } while (true);
}

/**
 * Function to manage the SD.
 */

void WaspUIO::menuSD()
{
  char str[80];

  SD.ON();

  do
  {
    cr.print();
    cr.print(F("1. List files"));
    cr.print(F("2. Show file"));
    cr.print(F("3. Format"));
    cr.print(F("9. Exit"));
    cr.print();
    input(str, sizeof(str), F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '1':
        SD.ls(LS_DATE | LS_SIZE | LS_R);
        break;
      case '2':
        input(str, sizeof(str), F("==> Enter path:"), 0);
        if (strlen(str) > 0)
        {
          SD.showFile((char*) str);
        }
        break;
      case '3':
        SD.format();
        break;
      case '9':
        SD.OFF();
        return;
    }
  } while (true);
}

/**
 * Function to configure the sensors.
 */

void WaspUIO::menuSensors()
{
  char str[80];

  do
  {
    cr.print();
#if USE_AGR
    cr.print(F("1. Agr board: Sensirion (%s)"), sensorStatus(sensor_sensirion));
    cr.print(F("2. Agr board: Pressure (%s)"), sensorStatus(sensor_pressure));
    cr.print(F("3. Agr board: Leaf wetness (%s)"), sensorStatus(sensor_leafwetness));
#endif
#if USE_I2C
    cr.print(F("4. I2C: BME-280 (%s)"), sensorStatus(sensor_bme280));
    cr.print(F("5. OneWire: DS1820 (%s)"), sensorStatus(sensor_ds1820));
#endif
#if USE_SDI
    cr.print(F("6. SDI-12: CTD-10 (%s)"), sensorStatus(sensor_ctd10));
    cr.print(F("7. SDI-12: DS-2 (%s)"), sensorStatus(sensor_ds2));
#endif
    cr.print(F("9. Exit"));
    cr.print();
    input(str, sizeof(str), F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '1':
        menuSensor(EEPROM_SENSOR_SENSIRION, sensor_sensirion);
        break;
      case '2':
        menuSensor(EEPROM_SENSOR_PRESSURE, sensor_pressure);
        break;
      case '3':
        menuSensor(EEPROM_SENSOR_LEAFWETNESS, sensor_leafwetness);
        break;
      case '4':
        menuSensor(EEPROM_SENSOR_BME280, sensor_bme280);
        break;
      case '5':
        menuSensor(EEPROM_SENSOR_DS1820, sensor_ds1820);
        break;
      case '6':
        menuSensor(EEPROM_SENSOR_CTD10, sensor_ctd10);
        break;
      case '7':
        menuSensor(EEPROM_SENSOR_DS2, sensor_ds2);
        break;
      case '9':
        cr.print();
        return;
    }
  } while (true);
}


/**
 * Function to configure how often the waspmote wakes up.
 */

void WaspUIO::menuWakeup()
{
  char str[80];
  int value;

  do
  {
    input(str, sizeof(str), F("Enter the wake period in minutes (1-255). Press Enter to leave it unchanged:"), 0);
    if (strlen(str) == 0)
      return;

    // Set new time
    if (sscanf(str, "%d", &value) == 1)
    {
      if (value >= 1 && value <= 255)
      {
        wakeup_period = (uint8_t) value;
        updateEEPROM(EEPROM_UIO_WAKEUP_PERIOD, wakeup_period);
        initTime();
        return;
      }
    }
  } while (true);
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
  char epochStr[11];  // Define variable for epoch string
  unsigned long epoch; // Define variable for epoch time

  debug(F("receiveGPSsyncTime: got frame %s"), buffer);

  pch = strstr(buffer,"TST:");
  //trace(F("receiveGPSsyncTime: pch = %s"), pch);
  memcpy(epochStr, &pch[4],10);
  epoch = atol(epochStr);
  epoch = epoch + 2; // Add some seconds for the delays, Check this!!!
  // Break Epoch time into UTC time
  RTC.breakTimeAbsolute(getEpochTime(), &time);

  // clear buffer and ...
  memset(buffer, 0x00, sizeof(buffer));
  sprintf(buffer, "%02u:%02u:%02u:%02u:%02u:%02u:%02u",
    time.year, time.month, time.date, time.day,
    time.hour, time.minute, time.second );

  // Setting time [yy:mm:dd:dow:hh:mm:ss]
  if (RTC.setTime(buffer) != 0) // '0' on succes, '1' otherwise
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
  } while (cr.millisDiff(start) < duration_ms);
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

/**
 * Write a value to the EEPROM only if different from the value already saved.
 *
 * Parameters:
 * - address    Address in the EEPROM, must be at least EEPROM_START (1024)
 * - value      Value to write
 *
 * Returns true if the operation was successful, false otherwise.
 *
 * These wraps some eeprom.h update functions. It adds: checks the address is
 * allowed, and returns whether the value was successfully saved.
 */

bool WaspUIO::updateEEPROM(int address, uint8_t value)
{
  if (address < EEPROM_START)
  {
    return false;
  }

  eeprom_update_byte((uint8_t*)address, value);
  return eeprom_read_byte((uint8_t*)address) == value;
}

bool WaspUIO::updateEEPROM(int address, uint32_t value)
{
  if (address < EEPROM_START)
  {
    return false;
  }

  eeprom_update_dword((uint32_t*)address, value);
  return eeprom_read_dword((uint32_t*)address) == value;
}

/**
 * Append data to the given file.
 *
 * Return the number of bytes written. Or -1 for error.
 */

int WaspUIO::append(SdFile &file, const void* buf, size_t nbyte)
{
  int n;

  if (file.seekEnd() == false)
  {
    error(F("append(%s): seekEnd failed"), tmpFilename);
    return -1;
  }

  n =file.write(buf, nbyte);
  if (n == -1)
  {
    error(F("append(%s): write failed"), tmpFilename);
    return -1;
  }

  return n;
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

  // Read temperature now that RTC is ON (it takes less than 2ms)
  rtc_temp = RTC.getTemperature();

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
  uint32_t diff = cr.millisDiff(start);

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
  char alarmTime[12]; // "00:00:00:00"

  // Enable RTC interruption and sleep
  getNextAlarm(alarmTime);
  PWR.deepSleep(alarmTime, RTC_ABSOLUTE, RTC_ALM1_MODE4, ALL_OFF);

  // Awake: disable RTC interruptions
  RTC.detachInt();
}


/***********************************************************************
 * Tasks
 ***********************************************************************/

/**
 * Internal sensors
 *
 * The accelerometer uses the I2C bus.
 */

CR_TASK(taskAcc)
{
  int16_t accX, accY, accZ;

  // ON
  ACC.ON();

  // Check correct operation
  if (ACC.check() != 0x32)
  {
    error(F("acc: check failed"));
    return CR_TASK_ERROR;
  }

  // Read values
  accX = ACC.getX();
  accY = ACC.getY();
  accZ = ACC.getZ();

  // OFF
  ACC.OFF();

  // Frame
  ADD_SENSOR(SENSOR_ACC, accX, accY, accZ);

  return CR_TASK_STOP;
}

CR_TASK(taskHealthFrame)
{
  // Frame: Device's health (battery, rtc temperature, acc)
  ADD_SENSOR(SENSOR_BAT, UIO.batteryLevel); // Battery level (uint8_t)
  ADD_SENSOR(SENSOR_IN_TEMP, UIO.rtc_temp); // RTC temperature in Celsius (float)

  return CR_TASK_STOP;
}


/**
 * Functions to turn on/off sensor power switches.
 * Supports the Agr board and the SDI-12 bus.
 *
 * The Agr board uses both 5V and 3V3. While the SDI-12 bus uses only 5V.
 */

CR_TASK(taskSensors)
{
#if USE_AGR
  bool agr = UIO.action(3, UIO.sensor_pressure, UIO.sensor_leafwetness, UIO.sensor_sensirion);
#endif
#if USE_SDI
  bool sdi = UIO.action(2, UIO.sensor_ctd10, UIO.sensor_ds2);
#endif
#if USE_I2C
  bool onewire = UIO.action(1, UIO.sensor_ds1820);
  bool i2c = UIO.action(1, UIO.sensor_bme280);
#endif
  static tid_t agr_id, sdi_id, onewire_id, i2c_id;

  CR_BEGIN;

  // Power On
#if USE_AGR
  if (agr)
  {
    info(F("Agr board ON"));
    SensorAgrv20.ON();
  }
#endif
#if USE_SDI
  if (! (WaspRegister & REG_5V) && sdi)
  {
    info(F("5V ON"));
    PWR.setSensorPower(SENS_5V, SENS_ON);
  }
#endif
#if USE_I2C
  if (! (WaspRegister & REG_3V3) && (onewire || i2c))
  {
    info(F("3V3 ON"));
    PWR.setSensorPower(SENS_3V3, SENS_ON);
  }
#endif

  // Init BME-280. Copied from BME280::ON to avoid the 100ms delay
  // TODO Do this once in the setup
  if (UIO.action(1, UIO.sensor_bme280))
  {
    // Check if the sensor is accesible
    if (BME.checkID() == 1)
    {
      // Read the calibration registers
      BME.readCalibration();
    }
  }

  // Wait for power to stabilize
  CR_DELAY(500);

  // Spawn tasks to take measures
#if USE_AGR
  if (agr)     { CR_SPAWN2(taskAgr, agr_id); }
#endif
#if USE_SDI
  if (sdi)     { CR_SPAWN2(taskSdi, sdi_id); }
#endif
#if USE_I2C
  if (onewire) { CR_SPAWN2(task1Wire, onewire_id); }
  if (i2c)     { CR_SPAWN2(taskI2C, i2c_id); }
#endif

  // Wait for tasks to complete
#if USE_AGR
  if (agr)     { CR_JOIN(agr_id); }
#endif
#if USE_SDI
  if (sdi)     { CR_JOIN(sdi_id); }
#endif
#if USE_I2C
  if (onewire) { CR_JOIN(onewire_id); }
  if (i2c)     { CR_JOIN(i2c_id); }
#endif

  // Power Off
#if USE_AGR
  if (agr)
  {
    info(F("Agr board OFF"));
    SensorAgrv20.OFF();
  }
#endif
  if (WaspRegister & REG_5V)
  {
    info(F("5V OFF"));
    PWR.setSensorPower(SENS_5V, SENS_OFF);
  }
  if (WaspRegister & REG_3V3)
  {
    info(F("3V3 OFF"));
    PWR.setSensorPower(SENS_3V3, SENS_OFF);
  }

  CR_END;
}

/**
 * The Agr board
 */

CR_TASK(taskAgr)
{
  static tid_t p_id, lc_id;

  CR_BEGIN;

  // Measure
  if (UIO.action(1, UIO.sensor_pressure))
  {
    CR_SPAWN2(taskAgrPressure, p_id);
  }
  if (UIO.action(2, UIO.sensor_leafwetness, UIO.sensor_sensirion))
  {
    CR_SPAWN2(taskAgrLC, lc_id);
  }

  // Wait
  if (UIO.action(1, UIO.sensor_pressure))
  {
    CR_JOIN(p_id);
  }
  if (UIO.action(2, UIO.sensor_leafwetness, UIO.sensor_sensirion))
  {
    CR_JOIN(lc_id);
  }

  CR_END;
}

CR_TASK(taskAgrPressure)
{
  float pressure;

  CR_BEGIN;

  SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_PRESSURE);   // On
  CR_DELAY(50);
  pressure = SensorAgrv20.readValue(SENS_AGR_PRESSURE); // Read
  SensorAgrv20.setSensorMode(SENS_OFF, SENS_AGR_PRESSURE);  // Off
  ADD_SENSOR(SENSOR_PA, pressure);

  CR_END;
}

/* The Low consumption group. */
CR_TASK(taskAgrLC)
{
  float temperature, humidity, wetness;

  CR_BEGIN;

  // Leaf wetness
  if (UIO.action(1, UIO.sensor_leafwetness))
  {
    debug(F("Agr Leaf Wetness ON"));
    SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_LEAF_WETNESS);
    CR_DELAY(50);
    wetness = SensorAgrv20.readValue(SENS_AGR_LEAF_WETNESS);
    ADD_SENSOR(SENSOR_LW, wetness);
  }

  // Sensirion (temperature, humidity)
  if (UIO.action(1, UIO.sensor_sensirion))
  {
    SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_SENSIRION);
    CR_DELAY(50);
    temperature = SensorAgrv20.readValue(SENS_AGR_SENSIRION, SENSIRION_TEMP);
    humidity = SensorAgrv20.readValue(SENS_AGR_SENSIRION, SENSIRION_HUM);
    ADD_SENSOR(SENSOR_TCB, temperature); // Add digital temperature
    ADD_SENSOR(SENSOR_HUMB, humidity); // Add digital humidity
  }

  // OFF
  if (UIO.action(1, UIO.sensor_leafwetness))
  {
    debug(F("Agr Leaf Wetness OFF"));
    SensorAgrv20.setSensorMode(SENS_OFF, SENS_AGR_LEAF_WETNESS);
  }
  if (UIO.action(1, UIO.sensor_sensirion))
  {
    SensorAgrv20.setSensorMode(SENS_OFF, SENS_AGR_SENSIRION);
  }

  CR_END;
}


/**
 * SDI-12
 */

CR_TASK(taskSdi)
{
  static tid_t ctd_id, ds_id;

  CR_BEGIN;
  mySDI12.begin();

  // Measure
  if (UIO.action(1, UIO.sensor_ctd10))
  {
    CR_SPAWN2(taskSdiCtd10, ctd_id);
  }
  if (UIO.action(1, UIO.sensor_ds2))
  {
    CR_SPAWN2(taskSdiDs2, ds_id);
  }

  // Wait
  if (UIO.action(1, UIO.sensor_ctd10))
  {
    CR_JOIN(ctd_id);
  }
  if (UIO.action(1, UIO.sensor_ds2))
  {
    CR_JOIN(ds_id);
  }

  mySDI12.end();
  CR_END;
}

CR_TASK(taskSdiCtd10)
{
  CR_BEGIN;

  // Send the measure command
  if (mySDI12.measure(0))
  {
    CR_ERROR;
  }

  // Wait (FIXME Get the value from the answer to the measure command)
  CR_DELAY(800);

  // Send the data command
  if (mySDI12.data(0))
  {
    CR_ERROR;
  }

  // Frame. The result looks like 0+167+17.5+103
  char *next;
  double a, b, c;

  a = strtod(mySDI12.buffer+1, &next);
  b = strtod(next, &next);
  c = strtod(next, &next);
  ADD_SENSOR(SENSOR_SDI12_CTD10, a, b, c);

  // Success
  CR_END;
}

CR_TASK(taskSdiDs2)
{
  CR_BEGIN;

  // Send the measure command
  if (mySDI12.command2address(1, "M6"))
  {
    CR_ERROR;
  }
  CR_DELAY(11000); // FIXME Should depend on response from M command

  // Wind speed&direction, air temp
  if (mySDI12.command2address(1, "D0"))
  {
    CR_ERROR;
  }

  // Frame. The result looks like 1+1.04+347+25.2+1.02-0.24+2.05
  char *next;
  double a, b, c;

  a = strtod(mySDI12.buffer+1, &next);
  b = strtod(next, &next);
  c = strtod(next, &next);
  ADD_SENSOR(SENSOR_SDI12_DS2_1, a, b, c);

  a = strtod(next, &next);
  b = strtod(next, &next);
  c = strtod(next, &next);
  ADD_SENSOR(SENSOR_SDI12_DS2_2, a, b, c);

  CR_END;
}

/**
 * OneWire
 */

CR_TASK(task1Wire)
{
  uint8_t addr[8];
  uint8_t data[12];
  uint8_t present, crc, n;
  int16_t temp;
  float temp_f;
  char temp_str[20];
  WaspOneWire oneWire(DIGITAL8); // pin hardcoded

  CR_BEGIN;

  // For now we only support the DS1820, so here just read that directly
  // We assume we have a chain of DS1820 sensors, and read all of them.

  present = oneWire.reset();
  if (! present)
  {
    error(F("OneWire no devices attached"));
    CR_ERROR;
  }

  // Send conversion command to all sensors
  oneWire.skip();
  oneWire.write(0x44, 1); // Keep sensors powered (parasite mode)
  CR_DELAY(1000);         // 750ms may be enough

  // TODO It may be better to search once in the setup, or in the menu, and
  // store the addresses in the EEPROM
  n = 0;
  oneWire.reset_search();
  while (oneWire.search(addr))
  {
    // Check address CRC
    crc = oneWire.crc8(addr, 7);
    if (crc != addr[7])
    {
      error(
        F("OneWire %02X%02X%02X%02X%02X%02X%02X%02X bad address, CRC failed: %02X"),
        addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7],
        crc
      );
      break;
    }

    // Check device type
    if (addr[0] == 0x28) // DS18B20
    {
      n++;
    }
    else
    {
      warn(
        F("OneWire %02X%02X%02X%02X%02X%02X%02X%02X unexpected device type: %02X"),
        addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7],
        addr[0]
      );
      continue;
    }

    // Read value
    present = oneWire.reset();
    oneWire.select(addr);
    oneWire.write(0xBE); // Read Scratchpad
    oneWire.read_bytes(data, 9); // We need 9 bytes

    crc = oneWire.crc8(data, 8);
    if (crc != data[8])
    {
      warn(
        F("OneWire %02X%02X%02X%02X%02X%02X%02X%02X bad data, CRC failed: %02X%02X%02X%02X%02X%02X%02X%02X%02X %02X"),
        addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7],
        data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8],
        crc
      );
    }

    // Convert to float. Formula for the DS18B20 model.
    temp = (data[1] << 8) | data[0];
    temp_f = (float) temp / 16;
    ADD_SENSOR(SENSOR_DS1820, temp_f);

    // Debug
    Utils.float2String(temp_f, temp_str, 2);
    debug(
      F("OneWire %02X%02X%02X%02X%02X%02X%02X%02X : %s"),
      addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7],
      temp_str
    );
  }

  debug(F("OneWire %d devices measured"), n);
  oneWire.depower();

  CR_END;
}

/**
 * I2C
 */

CR_TASK(taskI2C)
{
  float temperature, humidity, pressure;
  char aux[20];

  // Read enviromental variables
  temperature = BME.getTemperature(BME280_OVERSAMP_1X, 0);
  humidity = BME.getHumidity(BME280_OVERSAMP_1X);
  pressure = BME.getPressure(BME280_OVERSAMP_1X, 0);

  // Debug
  Utils.float2String(temperature, aux, 2);
  debug(F("BME-280 Temperature: %s Celsius Degrees"), aux);

  Utils.float2String(humidity, aux, 2);
  debug(F("BME-280 Humidity   : %s %%RH"), aux);

  Utils.float2String(pressure, aux, 2);
  debug(F("BME-280 Pressure   : %s Pa"), aux);

  // Frame
  ADD_SENSOR(SENSOR_BME_TC, temperature);
  ADD_SENSOR(SENSOR_BME_HUM, humidity);
  ADD_SENSOR(SENSOR_BME_PRES, pressure);

  return CR_TASK_STOP;
}


/**
 * Network
 */

CR_TASK(taskNetwork)
{
  static tid_t tid;

  // Send, once every 3 hours if low battery  and lithium battery
  bool send;
  if (UIO.batteryType == 1)
  {
    send = (
      UIO.hasSD &&
      (UIO.batteryLevel > 75) || (UIO.batteryLevel > 65 && UIO.time.hour % 3 == 0)
    );
  }

  // send period for Lead Acid battery
  else if (UIO.batteryType == 2)
  {
    send = UIO.hasSD;
  }

  CR_BEGIN;

  if (!xbeeDM.XBee_ON)
  {
    if (xbeeDM.ON())
    {
      error(F("startNetwork: xbeeDM.ON() failed"));
      CR_ERROR;
    }
  }
  info(F("Network started"));

  // Spawn first the receive task
  //CR_SPAWN(taskNetworkReceive);

  // Schedule sending frames
  if (send)
  {
    CR_SPAWN2(taskNetworkSend, tid);
  }

  CR_DELAY(8000); // Keep the network open at least for 8s

  if (send)
  {
    CR_JOIN(tid);
  }

  // Stop network
  if (xbeeDM.XBee_ON)
  {
    xbeeDM.OFF();
    info(F("Network stopped"));
  }

  CR_END;
}

CR_TASK(taskNetworkSend)
{
  SdFile archive;
  uint32_t fileSize;
  uint8_t item[8];
  uint32_t t0;
  char dataFilename[18]; // /data/YYMMDD.txt
  int size;

  CR_BEGIN;

  // Delay sending of frame by a random time within 50 to 550 ms to avoid
  // jaming the network.
  CR_DELAY(rand() % 500);

  // Security check, the file size must be a multiple of 8. If it is not we
  // consider there has been a write error, and we trunctate the file.
  fileSize = UIO.tmpFile.fileSize();
  if (fileSize % 8 != 0)
  {
    UIO.tmpFile.truncate(fileSize - fileSize % 8);
    warn(F("sendFrames: wrong file size (%s), truncated"), UIO.tmpFilename);
  }

  // Send frames
  while (UIO.tmpFile.fileSize())
  {
    t0 = millis();

    // Read the frame length
    UIO.tmpFile.seekEnd(-8);
    if (UIO.tmpFile.read(item, 8) != 8)
    {
      error(F("sendFrames (%s): read error"), UIO.tmpFilename);
      CR_ERROR;
    }

    // Read the frame
    UIO.getDataFilename(dataFilename, item[0], item[1], item[2]);
    if (!SD.openFile((char*)dataFilename, &archive, O_RDONLY))
    {
      error(F("sendFrames: fail to open %s"), dataFilename);
      CR_ERROR;
    }
    archive.seekSet(*(uint32_t *)(item + 3));
    size = archive.read(SD.buffer, (size_t) item[7]);
    archive.close();

    if (size < 0 || size != (int) item[7])
    {
      error(F("sendFrames: fail to read frame from disk %s"), dataFilename);
      CR_ERROR;
    }

    // Send the frame
    if (xbeeDM.send((char*)UIO.network.rx_address, (uint8_t*)SD.buffer, size) == 1)
    {
      warn(F("sendFrames: Send failure"));
      CR_ERROR;
    }

    // Truncate (pop)
    if (UIO.tmpFile.truncate(UIO.tmpFile.fileSize() - 8) == false)
    {
      error(F("sendFrames: error in tmpFile.truncate"));
      CR_ERROR;
    }

    debug(F("Frame sent in %lu ms"), cr.millisDiff(t0));

    // Give control back
    CR_DELAY(0);
  }

  CR_END;
}

CR_TASK(taskNetworkReceive)
{
  CR_BEGIN;

  while (xbeeDM.XBee_ON)
  {
    if (xbeeDM.available())
    {
      debug(F("receivePacket: data available"));
      // Data is expected to be available before calling this method, that's
      // why we only timout for 50ms, much less should be enough (to be
      // tested).
      if (xbeeDM.receivePacketTimeout(100))
      {
        warn(F("receivePacket: timeout (we will retry)"));
      }
      else
      {
        // Copy payload
        memset(UIO.buffer, 0x00, sizeof(UIO.buffer));
        memcpy(UIO.buffer, xbeeDM._payload, xbeeDM._length);

        // RSSI
        UIO.readRSSI2Frame();

        // Proxy call to appropriate handler
        if (strstr(UIO.buffer, "GPS_sync") != NULL)
        {
          UIO.receiveGPSsyncTime();
        }
        else
        {
          warn(F("receivePacket: unexpected packet"));
          // Show data stored in '_payload' buffer indicated by '_length'
          debug(F("Data: %s"), UIO.buffer);
          // Show data stored in '_payload' buffer indicated by '_length'
          debug(F("Length: %d"), xbeeDM._length);
          // Show data stored in '_payload' buffer indicated by '_length'
          debug(F("Source MAC Address: %02X%02X%02X%02X%02X%02X%02X%02X"),
                xbeeDM._srcMAC[0],
                xbeeDM._srcMAC[1],
                xbeeDM._srcMAC[2],
                xbeeDM._srcMAC[3],
                xbeeDM._srcMAC[4],
                xbeeDM._srcMAC[5],
                xbeeDM._srcMAC[6],
                xbeeDM._srcMAC[7]
               );
        }
      }
    }

    // Give control back
    CR_DELAY(0);
  }

  CR_END;
}

/**
 * Set RTC time from GPS
 *
 * TODO Use ephemiris, apparently this is required to improve power.
 */

CR_TASK(taskGps)
{
  if (GPS.ON() == 0)
  {
    warn(F("setTimeFromGPS: GPS.ON() failed, probably there is no GPS module"));
    return CR_TASK_ERROR;
  }

  if (GPS.waitForSignal() == false)
  {
    warn(F("setTimeFromGPS: Timeout"));
    GPS.OFF();
    return CR_TASK_ERROR;
  }

  if (GPS.getPosition() != 1)
  {
    warn(F("setTimeFromGPS: getPosition failed"));
    GPS.OFF();
    return CR_TASK_ERROR;
  }

  // Set time (XXX could optimize, as part of the work in setTimeFromGPS is
  // already done in getPosition above)
  GPS.setTimeFromGPS();
  UIO.initTime();
  info(F("setTimeFromGPS: Success, time updated"));

  // Location
  ADD_SENSOR(SENSOR_GPS,
             GPS.convert2Degrees(GPS.latitude , GPS.NS_indicator),
             GPS.convert2Degrees(GPS.longitude, GPS.EW_indicator));
  //ADD_SENSOR(SENSOR_ALTITUDE, GPS.altitude);
  //ADD_SENSOR(SENSOR_SPEED, GPS.speed);
  //ADD_SENSOR(SENSOR_COURSE, GPS.course);

  GPS.OFF();
  return CR_TASK_STOP;
}

/**
 * This task is only to test the watchdog reset.
 */

CR_TASK(taskSlow)
{
  CR_BEGIN;

  // Wait a little bit so this is executed last
  CR_DELAY(12000);

  warn(F("Start slow task"));
  delay(5 * 60000); // 5 minutes
  warn(F("End slow task"));

  CR_END;
}
