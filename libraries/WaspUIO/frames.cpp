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
const char frame_format_jjj   [] PROGMEM = "jjj";
const char frame_format_u     [] PROGMEM = "u";
//const char frame_format_v     [] PROGMEM = "v";
const char frame_format_w     [] PROGMEM = "w";
//const char frame_format_ww    [] PROGMEM = "ww";
const char frame_format_f     [] PROGMEM = "f";
const char frame_format_ff    [] PROGMEM = "ff";
const char frame_format_fff   [] PROGMEM = "fff";
const char frame_format_n     [] PROGMEM = "n";
const char frame_format_fffuf [] PROGMEM = "fffuf";
const char frame_format_uf    [] PROGMEM = "uf";
const char frame_format_jjk   [] PROGMEM = "jjk";
const char frame_format_jjjjjj[] PROGMEM = "jjjjjj";
const char frame_format_jjjjii[] PROGMEM = "jjjjii";

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
  // 06x
  null, null, null,
  frame_format_jjj,                                           // 63 Accelerometer
  null, null, null, null, null, null,
  // 07x
  null, null, null, null,
  null,                                                       // 74 Reserved (was BME temperature)
  null,
  null,                                                       // 76 Reserved (was BME humidity)
  null,                                                       // 77 Reserved (was BME pressure)
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
  null            ,     // 200 Reserved (was CTD-10)
  null,                 // 201 Reserved (was 201 DS-2 1)
  null,                 // 202 Reserved (was 201 DS-2 2)
  frame_format_n,       // 203 DS18B20
  null,                 // 204 Reserved (was MB73XX)
  frame_format_uf,      // 205 GPS number of satellites and accuracy
  frame_format_f,       // 206 Battery Volts
  frame_format_fffuf,   // 207 WS100-UMB
  null,                 // 208 Reserved (was DS-2)
  frame_format_fff,     // 209 BME 0x76 (internal)
  // 21x
  frame_format_fff,     // 210 BME 0x77
  frame_format_ff,      // 211 MLX90614
  frame_format_f,       // 212 TMP102
  frame_format_n,       // 213 VL53L1X
  frame_format_n,       // 214 MB73XX
  null,                 // 215 Reserved (was ATMOS-22)
  frame_format_jjk,     // 216 CTD-10
  frame_format_jjjjjj,  // 217 DS-2
  frame_format_jjjjii,  // 215 ATMOS-22
};

const char frame_name_bat         [] PROGMEM = "BAT";
const char frame_name_gps         [] PROGMEM = "GPS";
const char frame_name_rssi        [] PROGMEM = "RSSI";
const char frame_name_acc         [] PROGMEM = "ACC";
const char frame_name_altitude    [] PROGMEM = "ALT";
const char frame_name_tst         [] PROGMEM = "TST";
const char frame_name_ds18b20     [] PROGMEM = "DS18B20";
const char frame_name_gps_accuracy[] PROGMEM = "GPS Accuracy";
const char frame_name_bat_volts   [] PROGMEM = "BAT VOLTS";
const char frame_name_ws100       [] PROGMEM = "WS100";
const char frame_name_bme76       [] PROGMEM = "BME int";
const char frame_name_bme77       [] PROGMEM = "BME ext";
const char frame_name_mlx         [] PROGMEM = "MLX";
const char frame_name_tmp         [] PROGMEM = "TMP";
const char frame_name_vl          [] PROGMEM = "VL";
const char frame_name_mb73xx      [] PROGMEM = "MB73XX";
const char frame_name_ctd10       [] PROGMEM = "CTD10";
const char frame_name_ds2         [] PROGMEM = "DS-2";
const char frame_name_atmos       [] PROGMEM = "ATMOS";

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
  // 06x
  null, null, null,
  frame_name_acc,                                             // 63 Accelerometer
  null, null, null, null, null, null,
  // 07x
  null, null, null, null,
  null,                                                       // 74 Reserved (was BME temperature)
  null,
  null,                                                       // 76 Reserved (was BME humidity)
  null,                                                       // 77 Reserved (was BME pressure)
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
  null,                                                       // 200 Reserved (was CTD-10)
  null,                                                       // 201 Reserved (was DS-2 1)
  null,                                                       // 202 Reserved (was DS-2 2)
  frame_name_ds18b20,                                         // 203 DS18B20
  null,                                                       // 204 Reserved (was MB73XX)
  frame_name_gps_accuracy,                                    // 205 GPS Accuracy
  frame_name_bat_volts,                                       // 206 Battery Volts
  frame_name_ws100,                                           // 207 WS100-UMB
  null,                                                       // 208 Reserved (was DS-2)
  frame_name_bme76,                                           // 209 BME 0x76
  // 21x
  frame_name_bme77,                                           // 210 BME 0x77
  frame_name_mlx,                                             // 211 MLX90614
  frame_name_tmp,                                             // 212 TMP102
  frame_name_vl,                                              // 213 VL53L1X
  frame_name_mb73xx,                                          // 214 MB73XX
  null,                                                       // 215 Reserved (was ATMOS-22)
  frame_name_ctd10,                                           // 216 CTD-10
  frame_name_ds2,                                             // 217 DS-2
  frame_name_atmos,                                           // 218 ATMOS-22
};

/**
 * Wrap the frame.addSensor functions. If there is no place left in the frame,
 * saves the frame to the SD, creates a new one, and then adds the sensor to
 * the new frame.
 */

void WaspUIO::createFrame(bool discard)
{
  if (frame.numFields > 1 && !discard)
  {
    frame2Sd();
  }

  frame.createFrameBin(BINARY);
  addSensor(SENSOR_TST, _epoch);
}


/**
 * More flexible and memory efficient alternative to the addSensor functions in
 * upstream library.
 */

uint8_t WaspUIO::addSensorValue(uint8_t size, void* value)
{
  uint16_t new_size = frame.length + size;
  if (new_size > frameSize)
  {
    return 0;
  }

  memcpy(&(frame.buffer[frame.length]), value, size);
  frame.length += size;
  return size;
}

uint8_t WaspUIO::addSensorValue(float value)
{
  return addSensorValue(4, &value);
}

uint8_t WaspUIO::addSensorValue(int8_t value)
{
  return addSensorValue(1, &value);
}

uint8_t WaspUIO::addSensorValue(uint8_t value)
{
  return addSensorValue(1, &value);
}

uint8_t WaspUIO::addSensorValue(int16_t value)
{
  return addSensorValue(2, &value);
}

uint8_t WaspUIO::addSensorValue(uint16_t value)
{
  return addSensorValue(2, &value);
}

uint8_t WaspUIO::addSensorValue(int32_t value)
{
  return addSensorValue(4, &value);
}

uint8_t WaspUIO::addSensorValue(uint32_t value)
{
  return addSensorValue(4, &value);
}

int8_t WaspUIO::addSensor(uint8_t type, ...)
{
  va_list args;
  int8_t err = 0; // 0=ok -1=no-space -2=other-error
  uint8_t len = 0;
  uint16_t start;

  if (frame.numFields >= max_fields)
  {
    error(F("Max number of fields reached!"));
    return -2;
  }

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
  for (uint8_t i=0; i < strlen(format); i++)
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
      for (uint8_t j = 0; j < n; j++)
      {
        if (j > 0)
        {
          value = values[j] - values[j-1];
          if (value > -128 && value < 128)
          {
            if (addSensorValue((int8_t)value) == 0) { err = -1; goto exit; }
            continue;
          }

          // Use -128 as a marker to say next number is full 2 bytes
          if (addSensorValue((int8_t)-128) == 0) { err = -1; goto exit; }
        }

        if (addSensorValue(values[j]) == 0) { err = -1; goto exit; }
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
      debug(F("No space left in frame"));
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
  // Default to 255, will be used when there's not any network module
  payloadSize = 255;

#if WITH_4G
  // XXX Documentation says "Depends on the protocol used"
  if (networkType == NETWORK_4G) { payloadSize = 255; }
#endif

#if WITH_XBEE
  if (networkType == NETWORK_XBEE)
  {
    // We don't call frame.getMaxSizeForXBee to save memory, and because we
    // know already the value.
    // frame.getMaxSizeForXBee(DIGIMESH, addressing, DISABLED, encryption);
    payloadSize = 73;
#if WITH_CRYPTO
    if (strlen(password) > 0)
    {
      payloadSize = 48;
    }
#endif
  }
#endif

#if WITH_IRIDIUM
  if (networkType == NETWORK_IRIDIUM) { payloadSize = 340; }
#endif

  if (payloadSize > 255)
  {
    frameSize = 255;
  }
  else
  {
    frameSize = (uint8_t) payloadSize;
  }

  frame.setFrameSize(frameSize); // XXX Do we need this?
}


/**
 * Print the binary frame to USB.
 */

uint8_t WaspUIO::getSequence(uint8_t *p)
{
   uint8_t offset, i;

   // FIXME Encrypted frames don't have un-encrypted sequence number

   // Fixed header.
   // This only works with boot version G or above
   offset = 3 + 1 + 1 + 8;

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
   char c;
   uint8_t nfields;
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
   // This only works with boot version G or above
   Utils.hex2str(p, buffer, 8);
   p += 8;
   nbytes -= 8;
   cr.println(F("Serial ID: 0x%s"), buffer);

   // Waspmote ID
   for (uint8_t i = 0; i < 17 ; i++)
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
     uint8_t type = *p++;
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
       c = format[i];
       if (c == 'f')
       {
         Utils.float2String(*(float*)(void*)p, value_str, 4);
         cr.println(F("Sensor %d (%s): %s"), type, name, value_str);
         p += 4; nbytes -= 4;
       }
       else if (c == 'i')
       {
         cr.println(F("Sensor %d (%s): %d"), type, name, *(int8_t *)p);
         p += 1; nbytes -= 1;
       }
       else if (c == 'j')
       {
         cr.println(F("Sensor %d (%s): %d"), type, name, *(int16_t *)p);
         p += 2; nbytes -= 2;
       }
       else if (c == 'k')
       {
         cr.println(F("Sensor %d (%s): %ld"), type, name, *(int32_t *)p);
         p += 4; nbytes -= 4;
       }
       else if (c == 'u')
       {
         cr.println(F("Sensor %d (%s): %d"), type, name, *(uint8_t *)p);
         p += 1; nbytes -= 1;
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
 * Read a line from the given open file, not including the end-of-line
 * character. Store the read line in SD.buffer.
 *
 * Return the length of the line. Or -1 for EOF. Or -2 if error.
 */
void WaspUIO::getDataFilename(char* filename, uint8_t year, uint8_t month, uint8_t date)
{
  sprintf(filename, "%s/%02u%02u%02u.TXT", archive_dir, year, month, date);
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

#if WITH_CRYPTO
  // Encrypt frame
  if (strlen(password) > 0)
  {
    frame.encryptFrame(AES_128, password);
    if (flags & FLAG_LOG_USB)
    {
      frame.showFrame();
    }
  }
#endif

  // Start SD
  if (! hasSD) { return 1; }

  // (1) Get the date
  year = RTC.year;
  month = RTC.month;
  date = RTC.date;
  getDataFilename(dataFilename, year, month, date);

  // (2) Store frame in archive file
  if (sd_open(dataFilename, dataFile, O_WRITE | O_CREAT | O_APPEND))
  {
    error(F("Open data file failure"));
    return 1;
  }
  size = dataFile.fileSize();

  if (sd_append(dataFile, frame.buffer, frame.length))
  {
    dataFile.close();
    error(F("Append to data file failure"));
    return 1;
  }
  dataFile.close();

  // (3) Append to queue
  item[0] = year;
  item[1] = month;
  item[2] = date;
  *(uint32_t *)(item + 3) = size;
  item[7] = (uint8_t) frame.length;
#if WITH_IRIDIUM
  // This behaves like an action of type action_hours
  uint32_t hours = _epoch_minutes / 60;
  uint32_t minutes = _epoch_minutes % 60;
  if (SAVE_TO_LIFO_HOUR > 0
      && hours % (SAVE_TO_LIFO_HOUR * cooldown) == 0
      && minutes == SAVE_TO_LIFO_MINUTE)
  {
    if (lifo.push(item)) { return 2; }
    info(F("Frame saved to LIFO (%d fields in %d bytes)"), frame.numFields, frame.length);
  }
  else
  {
    if (fifo.push(item)) { return 2; }
    info(F("Frame saved to FIFO (%d fields in %d bytes)"), frame.numFields, frame.length);
  }
#else
  if (fifo.push(item)) { return 2; }
  info(F("Frame saved to FIFO (%d fields in %d bytes)"), frame.numFields, frame.length);
#endif


  return 0;
}


/**
 * This function reads the next frame from the SD.
 *
 * - Loads the frame into SD.buffer
 * - Returns the size of the frame, and the number of frames read
 *
 * If there's an error returns -1, if there're no frames returns 0.
 */
int WaspUIO::readFrame(uint8_t &n)
{
  uint8_t item[8];
  char dataFilename[18]; // /data/YYMMDD.txt
  SdFile dataFile;

  if (! hasSD)
  {
    return -1;
  }

  uint16_t maxSize = payloadSize;
  uint16_t totSize = 0;
  n  = 0;

#if WITH_IRIDIUM
  int idx = -1;
#else
  int idx = 0;
#endif

  while (true)
  {
#if WITH_IRIDIUM
    int status = lifo.peek(item, idx - n);
#else
    int status = fifo.peek(item, idx + n);
#endif
    if (status == QUEUE_EMPTY || status == QUEUE_INDEX_ERROR)
    {
      break;
    }
    else if (status)
    {
      error(F("readFrame peek(%d) failure"), idx);
      return -1;
    }

    // Stop condition
    int size = (int) item[7];
    if (totSize + size > maxSize)
    {
      break;
    }

    // Read the frame
    getDataFilename(dataFilename, item[0], item[1], item[2]);
    if (!SD.openFile((char*)dataFilename, &dataFile, O_READ))
    {
      error(F("readFrame fail to open %s"), dataFilename);
      return -1;
    }
    dataFile.seekSet(*(uint32_t *)(item + 3));
    uint8_t *start = (uint8_t*)&(SD.buffer[totSize]);
    int readSize = dataFile.read(start, (size_t) size);
    dataFile.close();

    if (readSize != size)
    {
      error(F("readFrame fail to read frame from disk %s"), dataFilename);
      return -1;
    }

    debug(F("frame seq=%hhu size=%d"), UIO.getSequence(start), size);

    totSize += size;
    n += 1;
  }

  return totSize;
}
