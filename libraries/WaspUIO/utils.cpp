#include "WaspUIO.h"


/* Sort in place integer array. Bubble sort. */
void sort_uint16(uint16_t* array, uint8_t size)
{
  bool done;
  uint16_t temp;
  uint8_t i;

  done = false;
  while (done == false)
  {
    done = true;
    for (i = 0; i < size; i++)
    {
      // numbers are out of order - swap
      if (array[i] > array[i+1])
      {
        temp = array[i+1];
        array [i+1] = array[i];
        array [i] = temp;
        done = false;
      }
    }
  }
}

/* Return median value of the given array. Modifies (sorts) the array. */
uint16_t median_uint16(uint16_t* array, uint8_t size)
{
  sort_uint16(array, size);

  if (size % 2 == 1)
  {
    return array[size/2];
  }
  else
  {
    return (array[size/2] + array[size/2 - 1]) / 2;
  }
}

/* Return standard deviation of the given array to the given value. */
uint16_t sd_uint16(uint16_t* array, uint8_t size, uint16_t mean)
{
  uint16_t value;
  uint32_t sd = 0;

  for (uint8_t i=0; i<size; i++)
  {
    value = array[i];
    value = (value > mean) ? (value - mean) : (mean - value);
    sd += (value * value);
  }

  return (uint16_t) sqrt(sd / size);
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
      if (cr.timeout(timeStart, timeout))
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
