#include "WaspUIO.h"


/**
 * Print the binary frame to USB.
 */

uint8_t WaspUIO::getSequence(uint8_t *p)
{
   uint8_t offset, i;

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

void WaspUIO::showBinaryFrame(uint8_t *p)
{
   uint8_t nbytes;
   char buffer[17];
   uint8_t i;
   char c;
   uint8_t sensor_id, nfields, type, decimals;
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
     sensor_id = *p++;
     nbytes--;
     // Use always the v15 tables
     // It would make sense to use the v12 only with a v12 board *and* one of
     // the Libelium's shields (eg the Agr board). But we don't support that
     // configuration.
     strcpy_P(name, (char*)pgm_read_word(&(FRAME_SENSOR_TABLE[sensor_id])));
     nfields = (uint8_t)pgm_read_word(&(FRAME_SENSOR_FIELD_TABLE[sensor_id]));
     type = (uint8_t)pgm_read_word(&(FRAME_SENSOR_TYPE_TABLE[sensor_id]));
     decimals = (uint8_t)pgm_read_word(&(FRAME_DECIMAL_TABLE[sensor_id]));

     if (nfields == 0)
     {
       nfields = *p++;
       nbytes--;
       // Special case, we store ints in a compressed format
       if (type == 1)
       {
         for (uint8_t j=0; j < nfields; j++)
         {
           if (j > 0)
           {
             diff = *(int8_t *)p; p++;
             nbytes--;
             if (diff != -128)
             {
               cr.println(F("Sensor %d (%s): %hhd"), sensor_id, name, diff);
               continue;
             }
           }

           cr.println(F("Sensor %d (%s): %d"), sensor_id, name, *(int *)p);
           p += 2; nbytes -= 2;
         }
         continue;
       }
     }

     for (; nfields > 0; nfields--)
     {
       if (type == 0) // uint8_t
       {
         cr.println(F("Sensor %d (%s): %d"), sensor_id, name, *p++);
         nbytes--;
       }
       else if (type == 1) // int
       {
         cr.println(F("Sensor %d (%s): %d"), sensor_id, name, *(int *)p);
         p += 2; nbytes -= 2;
       }
       else if (type == 2) // double
       {
         Utils.float2String(*(float *)p, value_str, decimals);
         cr.println(F("Sensor %d (%s): %s"), sensor_id, name, value_str);
         p += 4; nbytes -= 4;
       }
       else if (type == 3) // char*
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
         cr.println(F("Sensor %d (%s): %s"), sensor_id, name, value_str);
       }
       else if (type == 4) // uint32_t
       {
         cr.println(F("Sensor %d (%s): %lu"), sensor_id, name, *(uint32_t *)p);
         p += 4; nbytes -= 4;
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
  char format[10];
  const char* format_p;
  char c;
  int8_t err = 0; // 0=ok -1=no-space -2=other-error
  uint8_t len = 0;
  uint16_t start;

  // Read format from program memory
  format_p = FRAME_FORMAT_TABLE[type];
  if (format_p == NULL)
  {
    error(F("Unexpected frame type %hhu"), type);
    return -2;
  }
  strcpy_P(format, (char*)pgm_read_word(&format_p));

  // Type
  va_start(args, type);
  start = frame.length;
  if (addSensorValue(type) == 0) { err = -1; goto exit; }

  // Values
  for (int i=0; i<strlen(format); i++)
  {
    c = format[i];
    if (c == 'f')
    {
      float value = (float) va_arg(args, double);
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

  if (err != 0)
  {
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

  if (! hasSD)
  {
    return 1;
  }
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
    warn(F("sendFrames: wrong file size (%s), truncated"), UIO.queueFilename);
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

  // (4) Print frame to USB
  if (flags & FLAG_LOG_USB)
  {
    USB.ON();
    USB.flush();
    showBinaryFrame(frame.buffer);
    USB.OFF();
  }

  return 0;
}
