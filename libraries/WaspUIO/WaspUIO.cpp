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


SDI12 mySDI12(6); // DATAPIN = 6


/******************************************************************************
* Constructors
******************************************************************************/
//UIO::UIO()
//{

//}

// test!!


/******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * delFile
 *
 * Deletes the file with the given filename from the SD card. Returns 1 on
 * success, 0 on error.
 *
 * If the file does not exist it is not considered an error.
*/

uint8_t WaspUIO::delFile(const char* filename)
{
  if (SD.isFile(filename) == 1)
  {
    if(SD.del(filename))
    {
      info(F("%s deleted"), filename);
      return 1;
    }
    else
    {
      error(F("Failed to delete %s, error %d"), filename, SD.flag);
      return 0;
    }
  }

  debug(F("Failed to delete %s, it does not exist"), filename);
  return 1;
}

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
  switch (SD.isFile(filename))
  {
    case 1:
      trace(F("Failed to create %s, it already exists"), filename);
      return 0;
    case 0:
      error(F("Exists but not a file %s"), filename);
      break;
    case -1:
      if (SD.create(filename))
      {
        info(F("%s created"), filename);
        return 0;
      }
      else
      {
        error(F("Failed to create %s, error %d"), filename, SD.flag);
        return 1;
      }
  }

  // This should never happen
  error(F("Unexpected failure creating %s"), filename);
  return 1;
}

/**
 * Read information from EEPRROM and initialize variables.
 */
bool WaspUIO::initVars()
{
  // Flags
  flags = Utils.readEEPROM(EEPROM_UIO_FLAGS);
  featureUSB = flags & FLAG_LOG_USB;
  featureNetwork = flags & FLAG_NETWORK;

  // Network
  uint8_t panid_low = Utils.readEEPROM(EEPROM_UIO_NETWORK+1);
  if (panid_low > NETWORK_OTHER)
  {
    network = NULL;
  }
  else
  {
    network = &networks[panid_low];
    initNet((network_t) panid_low);
  }

  // Sensors
  sensors = eeprom_read_dword((const uint32_t *)EEPROM_UIO_SENSORS);

  // Log level
  cr.loglevel = (loglevel_t) Utils.readEEPROM(EEPROM_UIO_LOG_LEVEL);

  return true;
}

/**
 * Function to initialize SD card.
 * 1) check if there is an SD card.
 * 2) create files for program if non existing
 *
 * Returns 0 if success, 1 if error.
 *
 * TODO:
 * - include rules for when new datafiles will be created, eg. monthly files,
 *   max samples per fill etc...
 */

uint8_t WaspUIO::initSD()
{
  // Check for SD card
  if (! hasSD)
  {
    return 1;
  }

  // Create log file first, so logging is available
  //createFile(logFilename);

  // Create files used to send frames
  //createFile(tmpFilename);

  // Create archive directory
  switch (SD.isDir(archive_dir))
  {
    case 1:
      break;
    case 0:
      SD.del(archive_dir);
    case -1:
      SD.mkdir((char*)archive_dir);
  }

  return 0;
}

void WaspUIO::openFiles()
{
  if (hasSD)
  {
    // Open log file
    if (flags & FLAG_LOG_SD)
    {
      if (!SD.openFile((char*)logFilename, &logFile, O_CREAT | O_WRITE | O_APPEND | O_SYNC))
      {
        cr.print(F("ERROR start_RTC_SD_USB: opening log file failed"));
      }
    }
    // Open tmp file
    if (!SD.openFile((char*)tmpFilename, &tmpFile, O_CREAT | O_RDWR | O_APPEND | O_SYNC))
    {
      cr.print(F("ERROR start_RTC_SD_USB: opening tmp file failed"));
    }
  }
}

void WaspUIO::closeFiles()
{
  if (hasSD)
  {
    if (tmpFile.isOpen())
    {
      tmpFile.close();
    }
    if (logFile.isOpen())
    {
      logFile.close();
    }
  }
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
  network = &networks[value];
  if (
    updateEEPROM(EEPROM_UIO_NETWORK, network->panid[0]) &&
    updateEEPROM(EEPROM_UIO_NETWORK+1, network->panid[1])
  )
  {
    cr.print(F("INFO Network id saved to EEPROM"));
  }
  else
  {
    cr.print(F("ERROR Saving network id to EEPROM failed"));
  }

  initNet(value);
}

void WaspUIO::initNet(network_t value)
{
  uint8_t addressing = UNICAST_64B;

  // Check input parameter is valid
  if (value < NETWORK_FINSE || NETWORK_OTHER < value)
  {
      cr.print(F("ERROR No network configuration"));
      return;
  }

  // First enable network
  featureNetwork = 1;

  // Addressing
  network = &networks[value];
  cr.print(F("INFO Configure network: %s"), network->name);
  if (strcmp(network->rx_address, "000000000000FFFF") == 0)
  {
    addressing = BROADCAST_MODE;
  }

  // This is common to all networks, for now
  frame.setID((char*) "");
  encryptionMode = 0;
  //Utils.setAuthKey(key_access);

  // Set Frame size. Will be 73 bytes for XBeeDM-pro S1
  // linkEncryption = DISABLED (not supported by DIGIMESH, apparently)
  // AESEncryption = DISABLED
  frame.setFrameSize(DIGIMESH, addressing, DISABLED, DISABLED);
  cr.print(F("DEBUG Frame size is %d"), frame.getFrameSize());

  // init XBee
  xbeeDM.ON();
  delay(50);

  // Set channel, check AT commmand execution flag
  xbeeDM.setChannel(network->channel);
  if( xbeeDM.error_AT == 0 ) {
    cr.print(F("DEBUG Channel set OK")); // Implement string in print function
  } else {
    cr.print(F("ERORR with setChannel()"));
  }

  // set PANID, check AT commmand execution flag
  xbeeDM.setPAN(network->panid);
  if( xbeeDM.error_AT == 0 ) {
    cr.print(F("DEBUG PAN ID set OK"));
  } else {
    cr.print(F("ERROR calling 'setPAN()'"));
  }

  // set encryption mode (1:enable; 0:disable), check AT commmand execution flag
  xbeeDM.setEncryptionMode( encryptionMode );
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

/******************************************************************************/
/* Function to start SD, and USB.
*/

void WaspUIO::start_RTC_SD_USB(bool rtc)
{
  // RTC
  if (rtc)
  {
    RTC.ON();
  }

  // USB
  if (featureUSB)
  {
    USB.ON();
  }

  // SD
  SD.ON();
  hasSD = (SD.isSD() == 1);
  if (! hasSD)
  {
    SD.OFF();
    return;
  }
}

/******************************************************************************/
/* Function to stop SD, and USB.
*/
void WaspUIO::stop_RTC_SD_USB(void)
{
  if (RTC.isON)
  {
    RTC.OFF();
  }

  SD.OFF();

  if (featureUSB)
  {
    USB.OFF();
  }
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
    USB.print(buffer);
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

/******************************************************************************/
/* Function to delete logFilename if its size exceeds 50MB
*/
void WaspUIO::delLogActivity(void){
  // delete log file if larger than 50MB
  if(SD.getFileSize(logFilename) > 50000000)
  {
    delFile(logFilename);
    info(F("INFO New log file"));
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

  frame.createFrame(ASCII);
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
  uint8_t item[7];

  if (! hasSD)
  {
    warn(F("frame2Sd: SD card not available"));
    return 1;
  }

  // (1) Get the date
  item[0] = RTC.year;
  item[1] = RTC.month;
  item[2] = RTC.date;
  getDataFilename(item[0], item[1], item[2]);

  // (2) Store frame in archive file
  if (UIO.createFile(UIO.dataFilename))
  {
    error(F("frame2Sd: Failed to create %s"), UIO.dataFilename);
    return 1;
  }

  size = SD.getFileSize(UIO.dataFilename);
  if (size < 0)
  {
    error(F("frame2Sd: Failed to get size from %s"), UIO.dataFilename);
    return 1;
  }

  if (SD.appendln(UIO.dataFilename, frame.buffer, frame.length) == 0)
  {
    error(F("frame2Sd: Failed to append frame to %s"), UIO.dataFilename);
    return 1;
  }

  // (3) Append to queue
  *(uint32_t *)(item + 3) = (uint32_t) size;
  if (append(tmpFile, item, 7) == -1)
  {
    error(F("frame2Sd: Failed to append to %s"), tmpFilename);
    return 2;
  }

  // (4) Print frame to USB
  if (featureUSB)
  {
    frame.showFrame();
  }

  return 0;
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

const char* WaspUIO::input(const __FlashStringHelper * prompt_F, unsigned long timeout)
{
  static char buffer[100];
  char prompt[100];
  size_t max = sizeof(buffer) - 1;
  int i = 0;

  strncpy_F(prompt, prompt_F, sizeof(prompt));
  if (strlen(prompt))
  {
    cr.print(prompt);
  }
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
  for (i=0; i < max; i++)
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

void WaspUIO::menu()
{
  const char* str;
  char buffer[100];
  size_t size = sizeof(buffer);

  USB.ON();
  RTC.ON();

  // Go interactive or not
  if (input(F("Press Enter to start interactive mode. Wait 2 seconds to skip:"), 2000) == NULL)
  {
    cr.print();
    goto exit;
  }

  do {
    cr.print();
    cr.print(F("Battery: %d %%"), batteryLevel);
    if (! hasSD)
    {
      cr.print(F("SD card is missing!"));
    }
    cr.print();

    // Time
    cr.print(F("1. Time     : %s"), RTC.getTime());

    // Logging
    buffer[0] = '\0';
    if (flags & FLAG_LOG_USB) strnjoin_F(buffer, F("USB"), F(", "), size);
    if (flags & FLAG_LOG_SD)  strnjoin_F(buffer, F("SD"), F(", "), size);
    cr.print(F("2. Log      : level=%s output=%s"), cr.loglevel2str(cr.loglevel), buffer);

    // Network
    if (featureNetwork == 0)  strncpy_F(buffer, F("Disabled"), size);
    else if (network == NULL) strncpy_F(buffer, F("UNDEFINED"), size);
    else                      strncpy(buffer, network->name, size);
    cr.print(F("3. Network  : %s"), buffer);

    // Agr board
    buffer[0] = '\0';
    if (sensors & FLAG_AGR_SENSIRION)   strnjoin_F(buffer, F("Sensirion"), F(", "), size);
    if (sensors & FLAG_AGR_PRESSURE)    strnjoin_F(buffer, F("Pressure"), F(", "), size);
    if (sensors & FLAG_AGR_LEAFWETNESS) strnjoin_F(buffer, F("Leaf Wetness"), F(", "), size);
    if (! buffer[0])                    strncpy_F(buffer, F("(none)"), size);
    cr.print(F("4. Agr board: %s"), buffer);

    // SDI-12
    buffer[0] = '\0';
    if (sensors & FLAG_SDI12_CTD10)     strnjoin_F(buffer, F("CTD-10"), F(", "), size);
    if (sensors & FLAG_SDI12_DS2)       strnjoin_F(buffer, F("DS-2"), F(", "), size);
    if (! buffer[0])                    strncpy_F(buffer, F("(none)"), size);
    cr.print(F("5. SDI-12   : %s"), buffer);

    // SD
    if (hasSD)
    {
      cr.print(F("6. Open SD menu"));
    }

    cr.print(F("9. Exit"));
    cr.print();
    str = input(F("==> Enter numeric option:"), 0);
    switch (str[0])
    {
      case '1':
        menuTime();
        break;
      case '2':
        menuLog();
        break;
      case '3':
        menuNetwork();
        break;
      case '4':
        menuAgr();
        break;
      case '5':
        menuSDI12();
        break;
      case '6':
        if (hasSD)
	{
          SD.menu(30000);
          break;
        }
      case '9':
        cr.print();
        goto exit;
    }
  } while (true);

exit:
  // TODO Log configuration

  RTC.OFF();
  if (! featureUSB)
  {
    USB.OFF();
  }
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
  const char *str;

  do
  {
    cr.print();
    cr.print(F("1. Set time manually"));
    cr.print(F("2. Set time from GPS"));
    cr.print(F("9. Exit"));
    cr.print();
    str = input(F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '1':
        menuTimeManual();
        break;
      case '2':
        cr.print(F("Setting time from GPS, please wait, it may take a few minutes"));
        //actionGps();
        break;
      case '9':
        cr.print();
        return;
    }
  } while (true);
}


void WaspUIO::menuTimeManual()
{
  const char *str;
  unsigned short year, month, day, hour, minute, second;

  do
  {
    str = input(F("Set new time (format is yy:mm:dd:hh:mm:ss). Press Enter to leave it unchanged:"), 0);
    if (strlen(str) == 0)
      return;

    // Set new time
    if (sscanf(str, "%hu:%hu:%hu:%hu:%hu:%hu", &year, &month, &day, &hour, &minute, &second) == 6)
    {
      setTime(year, month, day, hour, minute, second);
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

void WaspUIO::menuAgr()
{
  const char *str;

  do
  {
    cr.print();
    cr.print(F("1. Agri board: Sensirion (%s)"), sensorStatus(FLAG_AGR_SENSIRION));
    cr.print(F("2. Agri board: Pressure (%s)"), sensorStatus(FLAG_AGR_PRESSURE));
    cr.print(F("3. Agri board: Leaf wetness (%s)"), sensorStatus(FLAG_AGR_LEAFWETNESS));
    cr.print(F("9. Exit"));
    cr.print();
    str = input(F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '1':
        menuAgrSensor(FLAG_AGR_SENSIRION);
        break;
      case '2':
        menuAgrSensor(FLAG_AGR_PRESSURE);
        break;
      case '3':
        menuAgrSensor(FLAG_AGR_LEAFWETNESS);
        break;
      case '9':
        cr.print();
        return;
    }
  } while (true);
}

const char* WaspUIO::sensorStatus(uint32_t sensor)
{
  return (sensors & sensor)? "enabled": "disabled";
}

void WaspUIO::menuAgrSensor(uint32_t sensor)
{
  const char *str;

  do
  {
    cr.print();
    cr.print(F("Current state is: %s"), sensorStatus(sensor));
    cr.print(F("1. Enable"));
    cr.print(F("2. Disable"));
    cr.print(F("9. Exit"));
    cr.print();
    str = input(F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '1':
        UIO.sensors |= sensor;
        updateEEPROM(EEPROM_UIO_SENSORS, UIO.sensors);
        cr.print();
        return;
      case '2':
        UIO.sensors &= ~sensor;
        updateEEPROM(EEPROM_UIO_SENSORS, UIO.sensors);
        cr.print();
        return;
      case '9':
        cr.print();
        return;
    }
  } while (true);
}

/**
 * Functions to manage SDI-12 sensors
 *
 * Parameters: void
 * Returns   : void
 *
 */

void WaspUIO::menuSDI12()
{
  const char *str;

  do
  {
    cr.print();
    cr.print(F("1. SDI-12: CTD-10 (%s)"), sensorStatus(FLAG_SDI12_CTD10));
    cr.print(F("2. SDI-12: DS-2 (%s)"), sensorStatus(FLAG_SDI12_DS2));
    cr.print(F("9. Exit"));
    cr.print();
    str = input(F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '1':
        menuSDI12Sensor(FLAG_SDI12_CTD10);
        break;
      case '2':
        menuSDI12Sensor(FLAG_SDI12_DS2);
        break;
      case '9':
        cr.print();
        return;
    }
  } while (true);
}

void WaspUIO::menuSDI12Sensor(uint32_t sensor)
{
  const char *str;

  cr.print(F("Enabling SDI-12"));
  PWR.setSensorPower(SENS_5V, SENS_ON);
  mySDI12.begin();

  do
  {
    cr.print();
    cr.print(F("Current state is: %s"), sensorStatus(sensor));
    cr.print(F("1. Enable"));
    cr.print(F("2. Disable"));
    cr.print(F("3. Identification"));
    cr.print(F("9. Exit"));
    cr.print();
    str = input(F("==> Enter numeric option:"), 0);
    if (strlen(str) == 0)
      continue;

    switch (str[0])
    {
      case '1':
        UIO.sensors |= sensor;
        updateEEPROM(EEPROM_UIO_SENSORS, UIO.sensors);
        cr.print();
        return;
      case '2':
        UIO.sensors &= ~sensor;
        updateEEPROM(EEPROM_UIO_SENSORS, UIO.sensors);
        cr.print();
        return;
      case '3':
        if (sensor == FLAG_SDI12_CTD10)
        {
          mySDI12.identification(0);
        }
        else if (sensor == FLAG_SDI12_DS2)
        {
          mySDI12.identification(1);
        }
        break;
      case '9':
        cr.print(F("Disabling SDI-12"));
        mySDI12.end();
        PWR.setSensorPower(SENS_5V, SENS_OFF);
        cr.print();
        return;
    }
  } while (true);
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
  const char *str;

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
    str = input(F("==> Enter numeric option:"), 0);
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
  const char *str;

  char* level;

  do
  {
    cr.print();
    cr.print(F("1. Log to SD (%s)"), flagStatus(FLAG_LOG_SD));
    cr.print(F("2. Log to USB (%s)"), flagStatus(FLAG_LOG_USB));
    cr.print(F("3. Choose the log level (%s)"), cr.loglevel2str(cr.loglevel));
    cr.print(F("9. Exit"));
    cr.print();
    str = input(F("==> Enter numeric option:"), 0);
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
  const char *str;

  do
  {
    cr.print(F("Type 1 to enable %s output, 0 to disable, Enter to leave:"), var);
    str = input(F(""), 0);
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

void WaspUIO::menuLogLevel()
{
  const char *str;

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
    str = input(F("==> Enter numeric option:"), 0);
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
  trace(F("receiveGPSsyncTime: pch = %s"), pch);
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
uint8_t WaspUIO::readMaxbotixSerial(uint8_t samples)
{
  const int bytes = 4; // Number of bytes to read
  char data_buffer[bytes]; // Store serial data
  int sample; // Store each sample


  Utils.setMuxAux1(); // check the manual to find out where you connect the sensor

  PWR.setSensorPower(SENS_3V3,SENS_ON); //  sensor needs 3.3 voltage
  delay(1000);

  beginSerial(9600,1); // set boud rate to 9600

  for (int j = 0; j < samples;)
  {
    // flush and wait for a range reading
    serialFlush(1);

    while (!serialAvailable(1) || serialRead(1) != 'R');

    // read the range
    for (int i = 0; i < bytes; i++)
    {
      while (!serialAvailable(1));

      data_buffer[i] = serialRead(1);
    }

    sample = (atoi(data_buffer));

    if (sample<=300 || sample>=5000)
    {
      debug(F("readMaxbotixSerial: NaN"));
      delay(10);
    }
    else
    {
      j++;
      debug(F("readMaxbotixSerial: sample = %d"), sample);
      // add a function for median value....  construct an array...
      delay(1000);
    }
  }
}

/******************************************************************************/
// Set defult to 5 samples
uint8_t WaspUIO::readMaxbotixSerial(void)
{
  return readMaxbotixSerial(5);
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
 * Read a line from the given open file, not including the end-of-line
 * character. Store the read line in SD.buffer.
 *
 * Return the length of the line. Or -1 for EOF. Or -2 if error.
 */

int WaspUIO::readline(SdFile &file)
{
  int n;

  n = file.fgets(SD.buffer, sizeof(SD.buffer));

  // Error
  if (n == -1)
    return -2;

  // EOF
  if (n == 0)
    return -1;

  if (SD.buffer[n - 1] == '\n') {
    SD.buffer[n - 1] = '\0';
    return n - 1;
  }

  return n;
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
void WaspUIO::getDataFilename(uint8_t year, uint8_t month, uint8_t date)
{
  sprintf(dataFilename, "%s/%02u%02u%02u.TXT", archive_dir, year, month, date);
}

/// Preinstantiate Objects /////////////////////////////////////////////////////
WaspUIO UIO = WaspUIO();


/******************************************************************************/
/* Function to read Mac Address
*/
const char* WaspUIO::readOwnMAC(void)
{
  static char mac[17];
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
  const char* ownMAC;
  char sourceMAC[17];
  uint8_t sourcePower;
  int rssi;
  uint8_t error;

  // 1. read own MAC address
  ownMAC = readOwnMAC();

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

    trace(F("readRSSI2Frame: getRSSI(dBm): %d"), rssi);
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

  // Read temperature now that RTC is ON (it takes less than 2ms)
  rtc_temp = RTC.getTemperature();

  // Turn off RTC only if it was off when this method was called
  if (! rtcON)
  {
    RTC.OFF();
  }

  // Battery
  batteryLevel = PWR.getBatteryLevel();
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
  return epochTime + cr.millisDiff(start) / 1000;
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
const char* WaspUIO::getNextAlarm(const uint8_t minute)
{
  uint8_t alarmMinute;
  static char alarmTime[12]; // "00:00:00:00"

  RTC.breakTimeAbsolute(UIO.getEpochTime(), &UIO.time);
  alarmMinute = (UIO.time.minute / minute) * minute + minute;
  if (alarmMinute >= 60)
    alarmMinute = 0;
  sprintf(alarmTime, "00:00:%02d:00", alarmMinute);

  return alarmTime;
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
  // ON
  ACC.ON();

  // Check correct operation
  if (ACC.check() != 0x32)
  {
    error(F("acc: check failed"));
    return CR_TASK_ERROR;
  }

  // Read values
  UIO.accX = ACC.getX();
  UIO.accY = ACC.getY();
  UIO.accZ = ACC.getZ();

  // OFF
  ACC.OFF();

  return CR_TASK_STOP;
}

CR_TASK(taskHealthFrame)
{
  // Frame: Device's health (battery, rtc temperature, acc)
  ADD_SENSOR(SENSOR_BAT, (uint8_t) PWR.getBatteryLevel()); // Battery level (uint8_t)
  ADD_SENSOR(SENSOR_IN_TEMP, UIO.rtc_temp); // RTC temperature in Celsius (float)
  //ADD_SENSOR(SENSOR_ACC, UIO.accX, UIO.accY, UIO.accZ); // Accelerometer

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
  bool agr = UIO.sensors & (FLAG_AGR_PRESSURE | FLAG_AGR_LEAFWETNESS | FLAG_AGR_SENSIRION);
  bool sdi = UIO.sensors & (FLAG_SDI12_CTD10 | FLAG_SDI12_DS2);
  static tid_t agr_id, sdi_id;

  CR_BEGIN;

  // Power On
  if (agr)
  {
    info(F("Agr board ON"));
    SensorAgrv20.ON();
  }
  else if (sdi)
  {
    info(F("5V ON"));
    PWR.setSensorPower(SENS_5V, SENS_ON);
  }

  // Wait for power to stabilize
  CR_DELAY(500);

  // Spawn tasks to take measures
  if (agr)
  {
    CR_SPAWN2(taskAgr, agr_id);
  }
  if (sdi)
  {
    CR_SPAWN2(taskSdi, sdi_id);
  }

  // Wait for tasks to complete
  if (agr)
  {
    CR_JOIN(agr_id);
  }
  if (sdi)
  {
    CR_JOIN(sdi_id);
  }

  // Power Off
  if (agr)
  {
    info(F("Agr board OFF"));
    SensorAgrv20.OFF();
  }
  else if (sdi)
  {
    info(F("5V OFF"));
    PWR.setSensorPower(SENS_5V, SENS_OFF);
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
  if (UIO.sensors & FLAG_AGR_PRESSURE)
  {
    CR_SPAWN2(taskAgrPressure, p_id);
  }
  if (UIO.sensors & (FLAG_AGR_LEAFWETNESS | FLAG_AGR_SENSIRION))
  {
    CR_SPAWN2(taskAgrLC, lc_id);
  }

  // Wait
  if (UIO.sensors & FLAG_AGR_PRESSURE)
  {
    CR_JOIN(p_id);
  }
  if (UIO.sensors & (FLAG_AGR_LEAFWETNESS | FLAG_AGR_SENSIRION))
  {
    CR_JOIN(lc_id);
  }

  CR_END;
}

CR_TASK(taskAgrPressure)
{
  CR_BEGIN;

  SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_PRESSURE);   // On
  CR_DELAY(50);
  UIO.pressure = SensorAgrv20.readValue(SENS_AGR_PRESSURE); // Read
  SensorAgrv20.setSensorMode(SENS_OFF, SENS_AGR_PRESSURE);  // Off
  ADD_SENSOR(SENSOR_PA, UIO.pressure);

  CR_END;
}

/* The Low consumption group. */
CR_TASK(taskAgrLC)
{
  CR_BEGIN;

  // Leaf wetness
  if (UIO.sensors & FLAG_AGR_LEAFWETNESS)
  {
    info(F("Agr Leaf Wetness ON"));
    SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_LEAF_WETNESS);
    CR_DELAY(50);
    UIO.wetness = SensorAgrv20.readValue(SENS_AGR_LEAF_WETNESS);
    ADD_SENSOR(SENSOR_LW, UIO.wetness);
    //ADD_SENSOR(SENSOR_TCC, DS18B20Temperature); // Add DS18B20 temperature
  }

  // Sensirion (temperature, humidity)
  if (UIO.sensors & FLAG_AGR_SENSIRION)
  {
    SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_SENSIRION);
    CR_DELAY(50);
    UIO.sensirionTemperature = SensorAgrv20.readValue(SENS_AGR_SENSIRION, SENSIRION_TEMP);
    UIO.sensirionHumidity = SensorAgrv20.readValue(SENS_AGR_SENSIRION, SENSIRION_HUM);
    ADD_SENSOR(SENSOR_TCB, UIO.sensirionTemperature); // Add digital temperature
    ADD_SENSOR(SENSOR_HUMB, UIO.sensirionHumidity); // Add digital humidity
  }

  // Soil temp (XXX Not yet implemented)
  if (false)
  {
    SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_TEMP_DS18B20);
    CR_DELAY(50);
    UIO.DS18B20Temperature = SensorAgrv20.readValue(SENS_AGR_TEMP_DS18B20);
  }

  // OFF
  if (UIO.sensors & FLAG_AGR_LEAFWETNESS)
  {
    info(F("Agr Leaf Wetness OFF"));
    SensorAgrv20.setSensorMode(SENS_OFF, SENS_AGR_LEAF_WETNESS);
  }
  if (UIO.sensors & FLAG_AGR_SENSIRION)
  {
    SensorAgrv20.setSensorMode(SENS_OFF, SENS_AGR_SENSIRION);
  }
  if (false) // XXX Not yet implemented
  {
    SensorAgrv20.setSensorMode(SENS_OFF, SENS_AGR_TEMP_DS18B20);
  }

  CR_END;
}

/* The Meteorology group. */
CR_TASK(taskAgrMeteo)
{
  CR_BEGIN;

  // Anemometer
  SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_ANEMOMETER);
  CR_DELAY(50);
  UIO.anemometer = SensorAgrv20.readValue(SENS_AGR_ANEMOMETER);

  // Vane
  SensorAgrv20.setSensorMode(SENS_ON, SENS_AGR_VANE);
  CR_DELAY(50);
  UIO.vane = SensorAgrv20.readValue(SENS_AGR_VANE);

  // Off
  SensorAgrv20.setSensorMode(SENS_OFF, SENS_AGR_VANE);
  SensorAgrv20.setSensorMode(SENS_OFF, SENS_AGR_ANEMOMETER);

  // Frame: anemometer, vane
  ADD_SENSOR(SENSOR_ANE, UIO.anemometer);  // Add anemometer value
  ADD_SENSOR(SENSOR_WV, UIO.vane);  // Add wind vane value

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
  if (UIO.sensors & FLAG_SDI12_CTD10)
  {
    CR_SPAWN2(taskSdiCtd10, ctd_id);
  }
  if (UIO.sensors & FLAG_SDI12_DS2)
  {
    CR_SPAWN2(taskSdiDs2, ds_id);
  }

  // Wait
  if (UIO.sensors & FLAG_SDI12_CTD10)
  {
    CR_JOIN(ctd_id);
  }
  if (UIO.sensors & FLAG_SDI12_DS2)
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
 * Network
 */

CR_TASK(taskNetwork)
{
  static tid_t tid;

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
  CR_SPAWN(taskNetworkReceive);

  // Schedule sending frames
  if (UIO.hasSD)
  {
    // Once every 3 hours if low battery
    if ((UIO.batteryLevel > 75) || (UIO.batteryLevel > 65 && UIO.time.hour % 3 == 0))
    {
      CR_SPAWN2(taskNetworkSend, tid);
    }
  }

  CR_DELAY(8000); // Keep the network open at least for 8s
  CR_JOIN(tid);

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
  uint8_t item[7];
  uint32_t t0;

  CR_BEGIN;

  // Delay sending of frame by a random time within 50 to 550 ms to avoid
  // jaming the network.
  CR_DELAY(rand() % 500);

  // Security check, the file size must be a multiple of 7. If it is not we
  // consider there has been a write error, and we trunctate the file.
  fileSize = UIO.tmpFile.fileSize();
  if (fileSize % 7 != 0)
  {
    UIO.tmpFile.truncate(fileSize - fileSize % 7);
    warn(F("sendFrames: wrong file size (%s), truncated"), UIO.tmpFilename);
  }

  // Send frames
  while (UIO.tmpFile.fileSize())
  {
    t0 = millis();

    // Read the frame length
    UIO.tmpFile.seekEnd(-7);
    if (UIO.tmpFile.read(item, 7) != 7)
    {
      error(F("sendFrames (%s): read error"), UIO.tmpFilename);
      CR_ERROR;
    }

    // Read the frame
    UIO.getDataFilename(item[0], item[1], item[2]);
    if (!SD.openFile((char*)UIO.dataFilename, &archive, O_RDONLY))
    {
      error(F("sendFrames: fail to open %s"), UIO.dataFilename);
      CR_ERROR;
    }
    archive.seekSet(*(uint32_t *)(item + 3));
    UIO.readline(archive);
    archive.close();

    // Send the frame
    if (xbeeDM.send((char*)UIO.network->rx_address, SD.buffer) == 1)
    {
      warn(F("sendFrames: Send failure"));
      CR_ERROR;
    }

    // Truncate (pop)
    if (UIO.tmpFile.truncate(UIO.tmpFile.fileSize() - 7) == false)
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
          warn(F("receivePacket: unexpected packet %s"), UIO.buffer);

          // Show data stored in '_payload' buffer indicated by '_length'
          cr.print(F("Data: %s"), UIO.buffer);
          // Show data stored in '_payload' buffer indicated by '_length'
          cr.print(F("Length: %d"), xbeeDM._length);
          // Show data stored in '_payload' buffer indicated by '_length'
          USB.print(F("Source MAC Address: "));
          USB.printHex( xbeeDM._srcMAC[0] );
          USB.printHex( xbeeDM._srcMAC[1] );
          USB.printHex( xbeeDM._srcMAC[2] );
          USB.printHex( xbeeDM._srcMAC[3] );
          USB.printHex( xbeeDM._srcMAC[4] );
          USB.printHex( xbeeDM._srcMAC[5] );
          USB.printHex( xbeeDM._srcMAC[6] );
          USB.printHex( xbeeDM._srcMAC[7] );
          cr.print();
          cr.print(F("--------------------------------"));
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
  info(F("setTimeFromGPS: Success, time updated"));

  // Location
  ADD_SENSOR(SENSOR_GPS,
             GPS.convert2Degrees(GPS.latitude , GPS.NS_indicator),
             GPS.convert2Degrees(GPS.longitude, GPS.EW_indicator));
  ADD_SENSOR(SENSOR_ALTITUDE, GPS.altitude);
  //ADD_SENSOR(SENSOR_SPEED, GPS.speed);
  //ADD_SENSOR(SENSOR_COURSE, GPS.course);

  GPS.OFF();
  return CR_TASK_STOP;
}
