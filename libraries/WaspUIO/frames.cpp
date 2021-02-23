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
const char frame_format_vvv   [] PROGMEM = "vvv";
const char frame_format_w     [] PROGMEM = "w";
const char frame_format_f     [] PROGMEM = "f";
const char frame_format_ff    [] PROGMEM = "ff";
const char frame_format_fff   [] PROGMEM = "fff";
const char frame_format_ffv   [] PROGMEM = "ffv";
const char frame_format_n     [] PROGMEM = "n";
const char frame_format_uf    [] PROGMEM = "uf";
const char frame_format_jjk   [] PROGMEM = "jjk";
const char frame_format_6j    [] PROGMEM = "jjjjjj";
const char frame_format_10v   [] PROGMEM = "vvvvvvvvvv";
const char frame_format_10f   [] PROGMEM = "ffffffffff";

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
  null,                 // 207 Reserved (was WS100-UMB)
  null,                 // 208 Reserved (was DS-2)
  frame_format_fff,     // 209 BME 0x76 (internal)
  // 21x
  frame_format_fff,     // 210 BME 0x77
  frame_format_ff,      // 211 MLX90614
  frame_format_f,       // 212 TMP102 & TMP117
  frame_format_n,       // 213 VL53L1X
  frame_format_n,       // 214 MB73XX
  null,                 // 215 Reserved (was ATMOS-22)
  frame_format_jjk,     // 216 CTD-10
  frame_format_6j,      // 217 DS-2
  frame_format_6j,      // 218 ATMOS-22
  frame_format_ff,      // 219 SHT31
  frame_format_10v,     // 220 AS7341
  frame_format_10f,     // 221 ICM20X
  frame_format_vvv,     // 222 VCNL4040
  frame_format_ffv,     // 223 VEML7700
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
const char frame_name_bme76       [] PROGMEM = "BME int";
const char frame_name_bme77       [] PROGMEM = "BME ext";
const char frame_name_mlx         [] PROGMEM = "MLX";
const char frame_name_tmp         [] PROGMEM = "TMP";
const char frame_name_vl          [] PROGMEM = "VL";
const char frame_name_mb73xx      [] PROGMEM = "MB73XX";
const char frame_name_ctd10       [] PROGMEM = "CTD10";
const char frame_name_ds2         [] PROGMEM = "DS-2";
const char frame_name_atmos       [] PROGMEM = "ATMOS";
const char frame_name_sht31       [] PROGMEM = "SHT31";
const char frame_name_as7341      [] PROGMEM = "AS7341";
const char frame_name_icm20x      [] PROGMEM = "ICM20X";
const char frame_name_vcnl4040    [] PROGMEM = "VCNL4040";
const char frame_name_veml7700    [] PROGMEM = "VEML7700";

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
  null,                                                       // 207 Reserved (was WS100-UMB)
  null,                                                       // 208 Reserved (was DS-2)
  frame_name_bme76,                                           // 209 BME 0x76
  // 21x
  frame_name_bme77,                                           // 210 BME 0x77
  frame_name_mlx,                                             // 211 MLX90614
  frame_name_tmp,                                             // 212 TMP102 & TMP117
  frame_name_vl,                                              // 213 VL53L1X
  frame_name_mb73xx,                                          // 214 MB73XX
  null,                                                       // 215 Reserved (was ATMOS-22)
  frame_name_ctd10,                                           // 216 CTD-10
  frame_name_ds2,                                             // 217 DS-2
  frame_name_atmos,                                           // 218 ATMOS-22
  frame_name_sht31,                                           // 219 SHT31
  frame_name_as7341,                                          // 220 AS7341
  frame_name_icm20x,                                          // 221 ICM20X
  frame_name_vcnl4040,                                        // 222 VCNL4040
  frame_name_veml7700,                                        // 223 VEML7700
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
    saveFrame();
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
  uint16_t start;

  if (frame.numFields >= max_fields) {
    log_error("Max number of fields reached!");
    return -2;
  }

  // Read format from program memory
  char format[11];
  strcpy_P(format, (char*)pgm_read_word(&(FRAME_FORMAT_TABLE[type])));
  if (strlen(format) == 0) {
    log_error("Unexpected frame type %hhu", type);
    return -2;
  }

  // Type
  va_start(args, type);
  start = frame.length;
  if (addSensorValue(type) == 0) { err = -1; goto exit; }

  // Values
  for (uint8_t i=0; i < strlen(format); i++) {
    char c = format[i];
    if (c == 'f') {
      float value = (float) va_arg(args, double);
      if (addSensorValue(value) == 0) { err = -1; goto exit; }
//  } else if (c == 'i') {
//    int8_t value = va_arg(args, int8_t);
//    if (addSensorValue(value) == 0) { err = -1; goto exit; }
    } else if (c == 'j') {
      int16_t value = va_arg(args, int16_t);
      if (addSensorValue(value) == 0) { err = -1; goto exit; }
    } else if (c == 'k') {
      int32_t value = va_arg(args, int32_t);
      if (addSensorValue(value) == 0) { err = -1; goto exit; }
    } else if (c == 'u') {
      uint8_t value = (uint8_t) va_arg(args, uint16_t);
      if (addSensorValue(value) == 0) { err = -1; goto exit; }
    } else if (c == 'v') {
      uint16_t value = va_arg(args, uint16_t);
      if (addSensorValue(value) == 0) { err = -1; goto exit; }
    } else if (c == 'w') {
      uint32_t value = va_arg(args, uint32_t);
      if (addSensorValue(value) == 0) { err = -1; goto exit; }
    } else if (c == 'n') {
      uint8_t n = (uint8_t) va_arg(args, uint16_t);
      if (addSensorValue(n) == 0) { err = -1; goto exit; }
      const int* values = va_arg(args, const int*);
      int32_t value;
      for (uint8_t j = 0; j < n; j++) {
        if (j > 0) {
          value = values[j] - values[j-1];
          if (value > -128 && value < 128) {
            if (addSensorValue((int8_t)value) == 0) { err = -1; goto exit; }
            continue;
          }

          // Use -128 as a marker to say next number is full 2 bytes
          if (addSensorValue((int8_t)-128) == 0) { err = -1; goto exit; }
        }

        if (addSensorValue(values[j]) == 0) { err = -1; goto exit; }
      }
    } else {
      log_error("Programming error: unexpected frame format %c", c);
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
      log_debug("No space left in frame");
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
  // The payload & frame sizes are set for outgoing messages, so WAN takes
  // priority over LAN

  // Set payloadSize
  // Avoid calling getMaxSizeForXXX functions to avoid using extra memory
  if (wan_type == WAN_4G) {
    payloadSize = 255;
  } else if (wan_type == WAN_IRIDIUM) {
    payloadSize = 340;
  } else if (lan_type == LAN_XBEE) {
    payloadSize = 73;
  } else if (lan_type == LAN_LORA) {
    // XXX MAX_PAYLOAD is 251 but I set 250 because from the packet description
    // (page 32) it looks to me it's 250, so better to be extra sure.
    payloadSize = 250;
  } else {
    // Default to 255, will be used when there's not any network module
    payloadSize = 255;
  }

  // Set frameSize
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
 * Return the sequence of the given frame.
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

   log_error("getSequence error");
   return 0;
}


/**
 * Parse and print frame. Return total length of the frame, 0 if error.
 */
uint16_t WaspUIO::parseFrame(uint8_t *p, uint16_t max_length)
{
    uint16_t length;
    // Header
    uint8_t frame_type;
    uint8_t nbytes;
    char serial[17];
    char mote_id[17];
    uint8_t seq;

    if (log_usb && cr.loglevel >= LOG_DEBUG)
        cr_printf("=== Binary Frame ===\n");

    // Start delimiter
    if (strncmp((const char*) p, "<=>", 3) != 0) {
        if (log_usb && cr.loglevel >= LOG_DEBUG)
            cr_printf("Error reading Start delimiter <=>\n");
        return 0;
    }
    p += 3;

    // Frame type
    frame_type = *p++;
    if (log_usb && cr.loglevel >= LOG_DEBUG)
        cr_printf("Frame type: %d\n", frame_type);
    // TODO Print text identifier: info, timeout, ...

    // Number of bytes
    nbytes = *p++;
    if (log_usb && cr.loglevel >= LOG_DEBUG)
        cr_printf("Number of bytes: %d\n", nbytes);

    length = 5 + nbytes;
    if (length > max_length) {
        if (log_usb && cr.loglevel >= LOG_DEBUG)
            cr_printf("Frame length (%u) greater than max-length (%u\n", length, max_length);
        return 0;
    }

    // Serial ID
    Utils.hex2str(p, serial, 8);
    p += 8;
    nbytes -= 8;
    if (log_usb && cr.loglevel >= LOG_DEBUG)
        cr_printf("Serial ID: 0x%s\n", serial);

    // Waspmote ID
    char c;
    for (uint8_t i = 0; i < 17 ; i++) {
        c = (char) *p++;
        nbytes--;
        if (c == '#') {
            mote_id[i] = '\0';
            break;
        }
        mote_id[i] = c;
    }
    if (log_usb && cr.loglevel >= LOG_DEBUG)
        cr_printf("Waspmote ID: %s\n", mote_id);

    // Separator
    if (c != '#') {
        if (log_usb && cr.loglevel >= LOG_DEBUG)
            cr_printf("Error reading Waspmote ID\n");
        return 0;
    }

    // Sequence
    seq = *p++;
    nbytes--;
    if (log_usb && cr.loglevel >= LOG_DEBUG)
        cr_printf("Sequence: %d\n", seq);

    // Payload
    while (nbytes > 0) {
        uint8_t type = *p++;
        nbytes--;

        // Read format string
        char format[11];
        strcpy_P(format, (char*)pgm_read_word(&(FRAME_FORMAT_TABLE[type])));
        if (strlen(format) == 0) {
            if (log_usb && cr.loglevel >= LOG_DEBUG)
                cr_printf("Unexpected frame type %hhu\n", type);
            return 0;
        }

        // Read name
        char name[20];
        strcpy_P(name, (char*)pgm_read_word(&(FRAME_NAME_TABLE[type])));

        // Values
        char value_str[50];
        for (size_t i=0; i < strlen(format); i++) {
            c = format[i];
            if (c == 'f') {
                Utils.float2String(*(float*)(void*)p, value_str, 4);
                if (log_usb && cr.loglevel >= LOG_DEBUG)
                    cr_printf("Sensor %d (%s): %s\n", type, name, value_str);
                p += 4; nbytes -= 4;
            } else if (c == 'i') {
                if (log_usb && cr.loglevel >= LOG_DEBUG)
                    cr_printf("Sensor %d (%s): %d\n", type, name, *(int8_t *)p);
                p += 1; nbytes -= 1;
            } else if (c == 'j') {
                if (log_usb && cr.loglevel >= LOG_DEBUG)
                    cr_printf("Sensor %d (%s): %d\n", type, name, *(int16_t *)p);
                p += 2; nbytes -= 2;
            } else if (c == 'k') {
                if (log_usb && cr.loglevel >= LOG_DEBUG)
                    cr_printf("Sensor %d (%s): %ld\n", type, name, *(int32_t *)p);
                p += 4; nbytes -= 4;
            } else if (c == 'u') {
                if (log_usb && cr.loglevel >= LOG_DEBUG)
                    cr_printf("Sensor %d (%s): %d\n", type, name, *(uint8_t *)p);
                p += 1; nbytes -= 1;
            } else if (c == 'v') {
                if (log_usb && cr.loglevel >= LOG_DEBUG)
                    cr_printf("Sensor %d (%s): %u\n", type, name, *(uint16_t *)p);
                p += 2; nbytes -= 2;
            } else if (c == 'w') {
                if (log_usb && cr.loglevel >= LOG_DEBUG)
                    cr_printf("Sensor %d (%s): %lu\n", type, name, *(uint32_t *)p);
                p += 4; nbytes -= 4;
            } else if (c == 'n') {
                uint8_t nfields = *p++;
                nbytes--;
                // Special case, we store ints in a compressed format
                for (uint8_t j=0; j < nfields; j++) {
                    if (j > 0) {
                        int8_t diff = *(int8_t *)p; p++;
                        nbytes--;
                        if (diff != -128) {
                            if (log_usb && cr.loglevel >= LOG_DEBUG)
                                cr_printf("Sensor %d (%s): %hhd\n", type, name, diff);
                            continue;
                        }
                    }

                    if (log_usb && cr.loglevel >= LOG_DEBUG)
                        cr_printf("Sensor %d (%s): %d\n", type, name, *(int *)p);
                    p += 2; nbytes -= 2;
                }
            } else if (c == 's') {
                uint8_t len = *p++;
                nbytes--;
                if (len > sizeof(value_str) - 1) {
                    if (log_usb && cr.loglevel >= LOG_DEBUG)
                        cr_printf("Error reading sensor value, string too long %d\n", len);
                    return 0;
                }
                strncpy(value_str, (char*) p, len);
                p += len; nbytes -= len;
                if (log_usb && cr.loglevel >= LOG_DEBUG)
                    cr_printf("Sensor %d (%s): %s\n", type, name, value_str);
            } else {
                if (log_usb && cr.loglevel >= LOG_DEBUG)
                    cr_printf("Sensor %d (%s): unexpected type %c\n", type, name, c);
            }
        }
    }

    if (log_usb && cr.loglevel >= LOG_DEBUG)
        cr_printf("====================\n");

    return length;
}


/**
 * Read a line from the given open file, not including the end-of-line
 * character. Store the read line in SD.buffer.
 *
 * Return the length of the line. Or -1 for EOF. Or -2 if error.
 */
uint8_t WaspUIO::getDataFilename(char* filename, uint8_t src, uint8_t year, uint8_t month, uint8_t date)
{
  if (src) {
    cr_sprintf(filename, "%s/%03u", archive_dir, src);
    if (sd_mkdir(filename))
    {
      log_error("getDataFilename fail to mkdir %s", filename);
      return 1;
    }
    cr_sprintf(filename, "%s/%03u/%02u%02u%02u.TXT", archive_dir, src, year, month, date);
  } else {
    cr_sprintf(filename, "%s/%02u%02u%02u.TXT", archive_dir, year, month, date);
  }

  return 0;
}


/**
 * Saves the frames in the given buffer. Returns the number of frames saved.
 */
uint8_t WaspUIO::saveFrames(uint8_t src, uint8_t *buffer, uint16_t max_length)
{
  uint8_t n = 0;
  uint8_t *p = buffer;
  uint16_t bytes_left = max_length;

  if (! hasSD)
  {
    return 0;
  }

  // Switch on USB
  if (log_usb)
  {
    USB.ON();
    USB.flush();
  }

  while (bytes_left > 0)
  {
    uint16_t length = parseFrame(p, bytes_left);
    if (length == 0)
    {
      break;
    }
    saveFrame(src, p, length);
    // Next
    p += length;
    bytes_left -= length;
    n++;
  }

  // Switch OFF USB
  USB.OFF();
  return n;
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
uint8_t WaspUIO::saveFrame(uint8_t src, uint8_t *buffer, uint16_t length)
{
  char dataFilename[21]; // /data/[SRC/]YYMMDD.txt (max length is 21)
  uint32_t size;
  uint8_t item[9];
  SdFile dataFile;

  // (1) Get the date
  timestamp_t ts;
  RTC.breakTimeAbsolute(_epoch, &ts);
  if (getDataFilename(dataFilename, src, ts.year, ts.month, ts.date)) { return 1; }

  // (2) Store frame in archive file
  if (sd_open(dataFilename, dataFile, O_WRITE | O_CREAT | O_APPEND | O_SYNC))
  {
    log_error("Open data file failure");
    return 1;
  }
  size = dataFile.fileSize();

  if (sd_append(dataFile, buffer, length))
  {
    dataFile.close();
    log_error("Append to data file failure");
    return 1;
  }
  dataFile.close();

  // (3) Append to queue
  item[0] = src;
  item[1] = ts.year;
  item[2] = ts.month;
  item[3] = ts.date;
  *(uint32_t *)(item + 4) = size;
  item[8] = (uint8_t) length;

  const char *queue_name = "FIFO";

#if WITH_IRIDIUM
  // This behaves like an action of type action_hours
  uint32_t hours = _epoch_minutes / 60;
  uint32_t minutes = _epoch_minutes % 60;
  if (SAVE_TO_LIFO_HOUR > 0
      && hours % (SAVE_TO_LIFO_HOUR * cooldown) == 0
      && minutes == SAVE_TO_LIFO_MINUTE)
  {
    queue_name = "LIFO";
    if (lifo.push(item)) { return 1; }
  }
  else
  {
    if (fifo.push(item)) { return 1; }
  }
#else
  if (fifo.push(item)) { return 1; }
#endif

  // Log
  log_info("Frame saved to %s bytes=%d", queue_name, length); // TODO Log frame sequence

  return 0;
}

uint8_t WaspUIO::saveFrame()
{
  if (! hasSD) { return 1; }

  // Print frame to USB for debugging
  if (log_usb)
  {
    USB.ON();
    USB.flush();
    parseFrame(frame.buffer, frame.length);
    USB.OFF();
  }

  return saveFrame(0, frame.buffer, frame.length);
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
  uint8_t item[9];
  char dataFilename[21]; // /data/[SRC/]YYMMDD.txt (max length is 21)
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
    int err = lifo.peek(item, idx - n);
#else
    int err = fifo.peek(item, idx + n);
#endif

    if (err == QUEUE_INDEX_ERROR) { break; }
    if (err) { return -1; }

    // Stop condition
    int size = (int) item[8];
    if (totSize + size > maxSize)
    {
      break;
    }

    // Read the frame
    if (getDataFilename(dataFilename, item[0], item[1], item[2], item[3])) { return -1; }
    if (!SD.openFile((char*)dataFilename, &dataFile, O_READ))
    {
      log_error("readFrame fail to open %s", dataFilename);
      return -1;
    }
    dataFile.seekSet(*(uint32_t *)(item + 4));
    uint8_t *start = (uint8_t*)&(SD.buffer[totSize]);
    int readSize = dataFile.read(start, (size_t) size);
    dataFile.close();

    if (readSize != size)
    {
      log_error("readFrame fail to read frame from disk %s", dataFilename);
      return -1;
    }

    log_debug("frame seq=%hhu size=%d", UIO.getSequence(start), size);

    totSize += size;
    n += 1;
  }

  return totSize;
}
