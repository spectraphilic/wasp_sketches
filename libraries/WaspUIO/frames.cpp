#include "WaspUIO.h"

/*
 * Frames
 * f: float
 * i: int8_t
 * j: int16_t
 * k: int32_t
 * u: uint8_t
 * v: uint16_t
 * w: uint32_t
 * n: int (multiple values)
 */


const char null               [] PROGMEM = "";
const char frame_format_j     [] PROGMEM = "j";
const char frame_format_u     [] PROGMEM = "u";
const char frame_format_w     [] PROGMEM = "w";
const char frame_format_ww    [] PROGMEM = "ww";
const char frame_format_f     [] PROGMEM = "f";
const char frame_format_ff    [] PROGMEM = "ff";
const char frame_format_fff   [] PROGMEM = "fff";
const char frame_format_n     [] PROGMEM = "n";
const char frame_format_fffuf [] PROGMEM = "fffuf";
const char frame_format_ffffff[] PROGMEM = "ffffff";

const char* const FRAME_FORMAT_TABLE[] PROGMEM = {
  null, null, null, null, null, null, null, null, null, null, // 00x
  null, null, null, null, null, null, null, null, null, null, // 01x
  null, null, null, null, null, null, null, null, null, null, // 02x
  null, null, null, null, null, null, null, null, null, null, // 03x
  null, null, null, null, null, null, null, null, null, null, // 04x
  // 05x
  null, null,
  frame_format_u,                                             // 52 Battery level
  frame_format_ff,                                            // 53 GPS
  frame_format_j,                                             // 54 RSSI
  null, null, null, null, null,
  null, null, null, null, null, null, null, null, null, null, // 06x
  // 07x
  null, null, null, null,
  frame_format_f,                                             // 74 BME temperature
  null,
  frame_format_f,                                             // 76 BME humidity
  frame_format_f,                                             // 77 BME pressure
  null, null,
  null, null, null, null, null, null, null, null, null, null, // 08x
  // 09x
  null,
  frame_format_f,                                             // 91 GPS Altitude
  null, null, null, null, null, null, null, null,
  null, null, null, null, null, null, null, null, null, null, // 10x
  null, null, null, null, null, null, null, null, null, null, // 11x
  // 12x
  null, null, null,
  frame_format_w,                                             // 123 Timestamp
  null, null, null, null, null, null,
  null, null, null, null, null, null, null, null, null, null, // 13x
  null, null, null, null, null, null, null, null, null, null, // 14x
  null, null, null, null, null, null, null, null, null, null, // 15x
  null, null, null, null, null, null, null, null, null, null, // 16x
  null, null, null, null, null, null, null, null, null, null, // 17x
  null, null, null, null, null, null, null, null, null, null, // 18x
  null, null, null, null, null, null, null, null, null, null, // 19x
  // 20x
  frame_format_fff,   // 200 CTD-10
  null,               // 201 Reserved, do not reuse (was 201 DS-2 1)
  null,               // 202 Reserved, do not reuse (was 201 DS-2 2)
  frame_format_n,     // 203 DS18B20
  frame_format_ww,    // 204 MB73XX
  frame_format_ww,    // 205 GPS statistics
  frame_format_f,     // 206 Battery Volts
  frame_format_fffuf, // 207 WS100-UMB
  frame_format_ffffff,// 208 DS-2
};

const char frame_name_bat      [] PROGMEM = "BAT";
const char frame_name_gps      [] PROGMEM = "GPS";
//const char frame_name_in_temp  [] PROGMEM = "IN TEMP";
const char frame_name_bme_tc   [] PROGMEM = "BME TC";
const char frame_name_bme_hum  [] PROGMEM = "BME HUM";
const char frame_name_bme_pres [] PROGMEM = "BME PRES";
const char frame_name_tst      [] PROGMEM = "TST";
const char frame_name_ctd10    [] PROGMEM = "CTD10";
//const char frame_name_ds2_1    [] PROGMEM = "DS2 1";
//const char frame_name_ds2_2    [] PROGMEM = "DS2 2";
const char frame_name_ds18b20  [] PROGMEM = "DS18B20";
const char frame_name_mb73xx   [] PROGMEM = "MB73XX";
const char frame_name_gps_stats[] PROGMEM = "GPS STATS";
const char frame_name_bat_volts[] PROGMEM = "BAT VOLTS";
const char frame_name_ws100    [] PROGMEM = "WS100";
const char frame_name_ds2      [] PROGMEM = "DS-2";
const char frame_name_rssi     [] PROGMEM = "RSSI";
const char frame_name_altitude [] PROGMEM = "ALT";

const char* const FRAME_NAME_TABLE[] PROGMEM=
{
  null, null, null, null, null, null, null, null, null, null, // 00x
  null, null, null, null, null, null, null, null, null, null, // 01x
  null, null, null, null, null, null, null, null, null, null, // 02x
  null, null, null, null, null, null, null, null, null, null, // 03x
  null, null, null, null, null, null, null, null, null, null, // 04x
  // 05x
  null, null,
  frame_name_bat,                                             // 52 Battery level
  frame_name_gps,                                             // 53 GPS
  frame_name_rssi,                                            // 54 RSSI
  null, null, null, null, null,
  null, null, null, null, null, null, null, null, null, null, // 06x
  // 07x
  null, null, null, null,
  frame_name_bme_tc,                                          // 74 BME temperature
  null,
  frame_name_bme_hum,                                         // 76 BME humidity
  frame_name_bme_pres,                                        // 77 BME pressure
  null, null,
  null, null, null, null, null, null, null, null, null, null, // 08x
  // 09x
  null,
  frame_name_altitude,                                        // 91 Altitude
  null, null, null, null, null, null, null, null,
  null, null, null, null, null, null, null, null, null, null, // 10x
  null, null, null, null, null, null, null, null, null, null, // 11x
  // 12x
  null, null, null,
  frame_name_tst,                                             // 123 Timestamp
  null, null, null, null, null, null,
  null, null, null, null, null, null, null, null, null, null, // 13x
  null, null, null, null, null, null, null, null, null, null, // 14x
  null, null, null, null, null, null, null, null, null, null, // 15x
  null, null, null, null, null, null, null, null, null, null, // 16x
  null, null, null, null, null, null, null, null, null, null, // 17x
  null, null, null, null, null, null, null, null, null, null, // 18x
  null, null, null, null, null, null, null, null, null, null, // 19x
  // 20x
  frame_name_ctd10,                                           // 200 CTD-10
  null,                                                       // 201 Reserved, do not reuse (was 201 DS-2 1)
  null,                                                       // 202 Reserved, do not reuse (was 201 DS-2 2)
  frame_name_ds18b20,                                         // 203 DS18B20
  frame_name_mb73xx,                                          // 204 MB73XX
  frame_name_gps_stats,                                       // 205 GPS statistics
  frame_name_bat_volts,                                       // 206 Battery Volts
  frame_name_ws100,                                           // 207 WS100-UMB
  frame_name_ds2,                                             // 208 DS-2
};

/**
 * Wrap the frame.addSensor functions. If there is no place left in the frame,
 * saves the frame to the SD, creates a new one, and then adds the sensor to
 * the new frame.
 */

void WaspUIO::createFrame()
{
  frame.createFrameBin(BINARY);
  addSensor(SENSOR_TST, epochTime);
}


/**
 * More flexible and memory efficient alternative to the addSensor functions in
 * upstream library.
 */

uint8_t WaspUIO::addSensorValue(float value)
{
  const uint8_t size = 4;
  if (frame.length + size <= frame.getFrameSize())
  {
    memcpy(&(frame.buffer[frame.length]), &value, size);
    frame.length += size;
    return size;
  }
  return 0;
}

uint8_t WaspUIO::addSensorValue(int8_t value)
{
  const uint8_t size = 1;
  if (frame.length + size <= frame.getFrameSize())
  {
    memcpy(&(frame.buffer[frame.length]), &value, size);
    frame.length += size;
    return size;
  }
  return 0;
}

uint8_t WaspUIO::addSensorValue(uint8_t value)
{
  const uint8_t size = 1;
  if (frame.length + size <= frame.getFrameSize())
  {
    memcpy(&(frame.buffer[frame.length]), &value, size);
    frame.length += size;
    return size;
  }
  return 0;
}

uint8_t WaspUIO::addSensorValue(int16_t value)
{
  const uint16_t size = 2;
  if (frame.length + size <= frame.getFrameSize())
  {
    memcpy(&(frame.buffer[frame.length]), &value, size);
    frame.length += size;
    return size;
  }
  return 0;
}

uint8_t WaspUIO::addSensorValue(uint16_t value)
{
  const uint16_t size = 2;
  if (frame.length + size <= frame.getFrameSize())
  {
    memcpy(&(frame.buffer[frame.length]), &value, size);
    frame.length += size;
    return size;
  }
  return 0;
}

uint8_t WaspUIO::addSensorValue(int32_t value)
{
  const uint16_t size = 4;
  if (frame.length + size <= frame.getFrameSize())
  {
    memcpy(&(frame.buffer[frame.length]), &value, size);
    frame.length += size;
    return size;
  }
  return 0;
}

uint8_t WaspUIO::addSensorValue(uint32_t value)
{
  const uint16_t size = 4;
  if (frame.length + size <= frame.getFrameSize())
  {
    memcpy(&(frame.buffer[frame.length]), &value, size);
    frame.length += size;
    return size;
  }
  return 0;
}

int8_t WaspUIO::addSensor(uint8_t type, ...)
{
  va_list args;
  int8_t err = 0; // 0=ok -1=no-space -2=other-error
  uint8_t len = 0;
  uint16_t start;

  // Read format from program memory
  char format[10];
  strcpy_P(format, (char*)pgm_read_word(&(FRAME_FORMAT_TABLE[type])));
  if (strlen(format) == 0)
  {
    error(F("Unexpected frame type %hhu"), type);
    return -2;
  }

  // Type
  va_start(args, type);
  start = frame.length;
  if (addSensorValue(type) == 0) { err = -1; goto exit; }

  // Values
  for (int i=0; i < strlen(format); i++)
  {
    char c = format[i];
    if (c == 'f')
    {
      float value = (float) va_arg(args, double);
      if (addSensorValue(value) == 0) { err = -1; goto exit; }
    }
//  else if (c == 'i')
//  {
//    int8_t value = va_arg(args, int8_t);
//    if (addSensorValue(value) == 0) { err = -1; goto exit; }
//  }
    else if (c == 'j')
    {
      int16_t value = va_arg(args, int16_t);
      if (addSensorValue(value) == 0) { err = -1; goto exit; }
    }
    else if (c == 'k')
    {
      int32_t value = va_arg(args, int32_t);
      if (addSensorValue(value) == 0) { err = -1; goto exit; }
    }
    else if (c == 'u')
    {
      uint8_t value = (uint8_t) va_arg(args, uint16_t);
      if (addSensorValue(value) == 0) { err = -1; goto exit; }
    }
    else if (c == 'v')
    {
      uint16_t value = va_arg(args, uint16_t);
      if (addSensorValue(value) == 0) { err = -1; goto exit; }
    }
    else if (c == 'w')
    {
      uint32_t value = va_arg(args, uint32_t);
      if (addSensorValue(value) == 0) { err = -1; goto exit; }
    }
    else if (c == 'n')
    {
      uint8_t n = (uint8_t) va_arg(args, uint16_t);
      if (addSensorValue(n) == 0) { err = -1; goto exit; }
      const int* values = va_arg(args, const int*);
      int32_t value;
      for (uint8_t i = 0; i < n; i++)
      {
        if (i > 0)
        {
          value = values[i] - values[i-1];
          if (value > -128 && value < 128)
          {
            if (addSensorValue((int8_t)value) == 0) { err = -1; goto exit; }
            continue;
          }

          // Use -128 as a marker to say next number is full 2 bytes
          if (addSensorValue((int8_t)-128) == 0) { err = -1; goto exit; }
        }

        if (addSensorValue(values[i]) == 0) { err = -1; goto exit; }
      }
    }
    else
    {
      error(F("Programming error: unexpected frame format %c"), c);
      err = -2;
      break;
    }
  }

exit:
  va_end(args);

  if (err)
  {
    if (err == -1)
    {
      error(F("No space left in frame"));
    }

    frame.length = start;
    //frame.buffer[frame.length] = '\0';
    return err;
  }

  frame.numFields++;                  // increment sensor fields counter
  frame.buffer[4] = frame.length - 5; // update number of bytes field

  // Add contents to struct (used for tiny frames)
//if (frame.numFields < frame.max_fields)
//{
//  frame.field[frame.numFields].flag = false;
//  frame.field[frame.numFields].start = start;
//  frame.field[frame.numFields].size = length - start;
//}

  //frame.buffer[frame.length] = '\0';
  return frame.length;
}


/*
 * Set frame size
 */
void WaspUIO::setFrameSize()
{
  // TODO Set size always to 255, then use frame fragmentation if needed

  if (networkType == NETWORK_XBEE)
  {
    // We don't call frame.getMaxSizeForXBee to save memory, and because we
    // know already the value.
    // frame.getMaxSizeForXBee(DIGIMESH, addressing, DISABLED, encryption);
    frame.setFrameSize(strlen(password) == 0 ? 73 : 48);
  }
  else // 4G
  {
    // XXX Likely less if encryption enabled
    frame.setFrameSize(255);
  }
}


/**
 * Print the binary frame to USB.
 */

uint8_t WaspUIO::getSequence(uint8_t *p)
{
   uint8_t offset, i;

   // FIXME Encrypted frames don't have un-encrypted sequence number

   // Fixed header, depends on version (4 or 8 bytes serial)
   if (_boot_version >= 'G') { offset = 3 + 1 + 1 + 8; }
   else                      { offset = 3 + 1 + 1 + 4; }

   // Variable header, mote name
   p += offset;
   for (i = 0; i < 17 ; i++)
   {
     if ((char)*p++ == '#')
     {
       return *p;
     }
   }

   error(F("getSequence error"));
   return 0;
}

void WaspUIO::showFrame(uint8_t *p)
{
   uint8_t nbytes;
   char buffer[17];
   uint8_t i;
   char c;
   uint8_t type, nfields;
   uint8_t len;
   char name[20];
   char value_str[50];
   int8_t diff;

   // Binary Frame
   cr.println(F("=== Binary Frame: %d fields in %d bytes ==="), frame.numFields, frame.length);

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
   //println(F("Number of bytes left: %d"), nbytes);

   // Serial ID
   //println(F("BOOT VERSION %c"), _boot_version);
   if (_boot_version >= 'G')
   {
     Utils.hex2str(p, buffer, 8);
     p += 8;
     nbytes -= 8;
   }
   else
   {
     Utils.hex2str(p, buffer, 4);
     p += 4;
     nbytes -=4;
   }
   cr.println(F("Serial ID: 0x%s"), buffer);

   // Waspmote ID
   for (i = 0; i < 17 ; i++)
   {
     c = (char) *p++;
     nbytes--;
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
   nbytes--;

   // Payload
   while (nbytes > 0)
   {
     type = *p++;
     nbytes--;

     // Read format string
     char format[10];
     strcpy_P(format, (char*)pgm_read_word(&(FRAME_FORMAT_TABLE[type])));
     if (strlen(format) == 0)
     {
       cr.println(F("Unexpected frame type %hhu"), type);
       return;
     }

     // Read name
     strcpy_P(name, (char*)pgm_read_word(&(FRAME_NAME_TABLE[type])));

     // Values
     for (int i=0; i < strlen(format); i++)
     {
       char c = format[i];
       if (c == 'f')
       {
         Utils.float2String(*(float *)p, value_str, 4);
         cr.println(F("Sensor %d (%s): %s"), type, name, value_str);
         p += 4; nbytes -= 4;
       }
       else if (c == 'j')
       {
         cr.println(F("Sensor %d (%s): %d"), type, name, *(int16_t *)p);
         p += 2; nbytes -= 2;
       }
       else if (c == 'u')
       {
         cr.println(F("Sensor %d (%s): %d"), type, name, *p++);
         nbytes--;
       }
       else if (c == 'v')
       {
         cr.println(F("Sensor %d (%s): %lu"), type, name, *(uint16_t *)p);
         p += 2; nbytes -= 2;
       }
       else if (c == 'w')
       {
         cr.println(F("Sensor %d (%s): %lu"), type, name, *(uint32_t *)p);
         p += 4; nbytes -= 4;
       }
       else if (c == 'n')
       {
         nfields = *p++;
         nbytes--;
         // Special case, we store ints in a compressed format
         for (uint8_t j=0; j < nfields; j++)
         {
           if (j > 0)
           {
             diff = *(int8_t *)p; p++;
             nbytes--;
             if (diff != -128)
             {
               cr.println(F("Sensor %d (%s): %hhd"), type, name, diff);
               continue;
             }
           }

           cr.println(F("Sensor %d (%s): %d"), type, name, *(int *)p);
           p += 2; nbytes -= 2;
         }
       }
       else if (c == 's')
       {
         len = *p++;
         nbytes--;
         if (len > sizeof(value_str) - 1)
         {
           cr.println(F("Error reading sensor value, string too long %d"), len);
           return;
         }
         strncpy(value_str, (char*) p, len);
         p += len; nbytes -= len;
         cr.println(F("Sensor %d (%s): %s"), type, name, value_str);
       }
       else
       {
         cr.println(F("Sensor %d (%s): unexpected type %c"), type, name, c);
       }
     }
   }

   cr.println(F("=========================================="));
}


/**
 * This function:
 *
 * - Stores the frame in the "archive file"
 * - Appends record to the FIFO queue (tmp file), to be sent later
 *
 * The data files are stored in /data/YYMMDD.TXT
 *
 * The FIFO file starts with a 4 bytes header. It contains the offset of the
 * next record to be sent, the start of the FIFO queue.
 *
 * After the header there is a sequence of 8 bytes records:
 *
 * - 0: year
 * - 1: month
 * - 2: date
 * - 3-6: offset in the archive file
 * - 7: the size of the frame
 *
 * Returns: uint8_t
 * - 0 success
 * - 1 error, failed to archive frame
 * - 2 error, archived frame, but append to queue failed
 */

uint8_t WaspUIO::frame2Sd()
{
  uint8_t year, month, date;
  char dataFilename[18]; // /data/YYMMDD.txt
  uint32_t size;
  uint8_t item[8];
  SdFile dataFile;

  // Print frame to USB for debugging
  if (flags & FLAG_LOG_USB)
  {
    USB.ON();
    USB.flush();
    showFrame(frame.buffer);
    USB.OFF();
  }

  // TODO Fragmentation

  // Encrypt frame
  //if (strlen(password) != 0)
  if (false)
  {
    frame.encryptFrame(AES_128, password);
    if (flags & FLAG_LOG_USB)
    {
      frame.showFrame();
    }
  }

  // Start SD
  if (! hasSD) { return 1; }
  startSD();

  // (1) Get the date
  year = RTC.year;
  month = RTC.month;
  date = RTC.date;
  getDataFilename(dataFilename, year, month, date);

  // (2) Store frame in archive file
  if (openFile(dataFilename, dataFile, O_WRITE | O_CREAT | O_APPEND))
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
  if (openFile(queueFilename, queueFile, O_RDWR | O_CREAT))
  {
    error(cr.last_error);
    return 2;
  }

  // Security check, the file size must be a multiple of 8. If it is not we
  // consider there has been a write error, and we trunctate the file.
  uint32_t offset = queueFile.fileSize() % 8;
  if (offset != 0)
  {
    queueFile.truncate(queueFile.fileSize() - offset);
    warn(F("sendFrames: wrong file size (%s), truncated"), queueFilename);
  }

  // Append record
  item[0] = year;
  item[1] = month;
  item[2] = date;
  *(uint32_t *)(item + 3) = size;
  item[7] = (uint8_t) frame.length;
  if (append(queueFile, item, 8))
  {
    error(cr.last_error);
    return 2;
  }

  return 0;
}


/**
 * This function reads the next frame from the SD.
 *
 * - Loads the frame into SD.buffer
 * - Returns the size of the frame
 *
 * If there's an error returns -1, if there're no frames returns 0.
 */
int WaspUIO::readFrame()
{
  static uint32_t offset;
  uint8_t item[8];
  char dataFilename[18]; // /data/YYMMDD.txt
  SdFile dataFile;
  int size;

  // Open files
  startSD();
  if (openFile(qstartFilename, qstartFile, O_READ)) { return -1; }
  if (openFile(queueFilename, queueFile, O_READ)) { return -1; }

  // Read offset
  if (qstartFile.read(item, 4) != 4)
  {
    cr.set_last_error(F("sendFrames (%s): read error"), qstartFilename);
    return -1;
  }
  offset = *(uint32_t *)item;
  if (offset >= queueFile.fileSize())
  {
    return 0; // Nothing to do
  }

  // Read the record
  queueFile.seekSet(offset);
  if (queueFile.read(item, 8) != 8)
  {
    cr.set_last_error(F("sendFrames (%s): read error"), queueFilename);
    return -1;
  }

  // Read the frame
  getDataFilename(dataFilename, item[0], item[1], item[2]);
  if (!SD.openFile((char*)dataFilename, &dataFile, O_READ))
  {
    cr.set_last_error(F("sendFrames: fail to open %s"), dataFilename);
    return -1;
  }
  dataFile.seekSet(*(uint32_t *)(item + 3));
  size = dataFile.read(SD.buffer, (size_t) item[7]);
  dataFile.close();

  if (size < 0 || size != (int) item[7])
  {
    cr.set_last_error(F("sendFrames: fail to read frame from disk %s"), dataFilename);
    return -1;
  }

  return size;
}
