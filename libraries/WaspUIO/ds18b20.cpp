/**
 * Functions to read strings of DS18B20 sensors.
 *
 */

#include "WaspUIO.h"


/** readDS18B20
 *
 * Returns: number of values read, -1 if error
 */
uint8_t WaspUIO::readDS18B20(int values[], uint8_t max)
{
  SdFile file;
  int len;
  char word[20];
  uint8_t pin;
  uint8_t n = 0;
  uint8_t addr[8];
  uint8_t data[12];
  uint8_t crc;
  int16_t temp;
  float temp_f;
  char temp_str[20];
  char data_str[17];

  if (SD.openFile("onewire.txt", &file, O_READ) == 0)
  {
    error(F("Missing onewire.txt file"));
    return 0;
  }

  bool old_state = pwr_1wire(1);

  while ((len = file.fgets(word, sizeof(word), (char*)" \n")) > 0)
  {
    bool eol = (word[--len] == '\n');
    if (eol)
    {
      continue;
    }

    word[len] = '\0'; // Remove delimiter
    if (len == 1)
    {
      pin = (uint8_t) atoi(word);
      WaspOneWire oneWire(_getPin(pin));
      if (! oneWire.reset())
      {
        warn(F("OneWire no devices attached to %hhu pin"), pin);
        continue;
      }

      // Send conversion command to all sensors
      oneWire.skip();
      oneWire.write(0x44, 1); // Keep power on (parasite mode)
      delay(800);             // Time for the sensors to do their job

      while (!eol && (len = file.fgets(word, sizeof(word), (char*)" \n")) > 0)
      {
        if (n >= max)
        {
          warn(F("OneWire cannot read more than %hhu sensors"), max);
          break;
        }

        eol = (word[--len] == '\n');
        word[len] = '\0'; // Remove delimiter
        if (len == 16)
        {
          if (Utils.str2hex(word, addr, 8) < 8)
          {
            warn(F("OneWire(%hhu) bad address '%s'"), pin, word);
            continue;
          }

          // Read value
          oneWire.reset();
          oneWire.select(addr);
          oneWire.write(0xBE); // Read Scratchpad
          oneWire.read_bytes(data, 9); // We need 9 bytes

          crc = oneWire.crc8(data, 8);
          if (crc == data[8])
          {
            temp = (data[1] << 8) | data[0];

            // Debug
            temp_f = (float) temp / 16;
            Utils.float2String(temp_f, temp_str, 3);
            debug(F("OneWire(%hhu) %s=%d (%s)"), pin, word, temp, temp_str);
          }
          else
          {
            temp = INT_MIN;
            Utils.hex2str(data, data_str, 8);
            warn(F("OneWire(%hhu) %s=%d (CRC failed: %s %02X)"), pin, temp, word, data_str, crc);
          }
          values[n++] = temp;
        }
      }
      oneWire.depower();
    }
  }

  pwr_1wire(old_state);

  file.close();
  if (len == -2)
  {
    warn(F("Could not close onewire.txt file"));
  }

  return n;
}
