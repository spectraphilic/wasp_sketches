#include "WaspUIO.h"


/* Sort in place integer array. Bubble sort. */
void WaspUIO::sort_uint16(uint16_t* array, uint8_t size)
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
uint16_t WaspUIO::median_uint16(uint16_t* array, uint8_t size)
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
uint16_t WaspUIO::sd_uint16(uint16_t* array, uint8_t size, uint16_t mean)
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
