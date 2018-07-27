#include "WaspUIO.h"

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
    error(F("cannot save value to lower eeprom memory address"));
    return false;
  }

  eeprom_update_byte((uint8_t*)address, value);
  bool ok = eeprom_read_byte((uint8_t*)address) == value;
  if (! ok)
  {
    error(F("value read from eeprom different from value written"));
  }

  return ok;
}

bool WaspUIO::updateEEPROM(int address, uint16_t value)
{
  if (address < EEPROM_START)
  {
    error(F("cannot save value to lower eeprom memory address"));
    return false;
  }

  eeprom_update_word((uint16_t*)address, value);
  bool ok = eeprom_read_word((uint16_t*)address) == value;
  if (! ok)
  {
    error(F("value read from eeprom different from value written"));
  }

  return ok;
}

bool WaspUIO::updateEEPROM(int address, uint32_t value)
{
  if (address < EEPROM_START)
  {
    error(F("cannot save value to lower eeprom memory address"));
    return false;
  }

  eeprom_update_dword((uint32_t*)address, value);
  bool ok = eeprom_read_dword((uint32_t*)address) == value;
  if (! ok)
  {
    error(F("value read from eeprom different from value written"));
  }

  return ok;
}

char* WaspUIO::readEEPROM(int address, char* dst, size_t size)
{
  size_t i;
  char c;

  for (i=0; i<size; i++)
  {
    c = (char)eeprom_read_byte((uint8_t*)(address + i));
    *(dst + i) = c;
    if (c == '\0')
    {
      break;
    }
  }

  if (i == size)
  {
    *(dst+size-1) = '\0';
  }

  return dst;
}

bool WaspUIO::writeEEPROM(int address, char* src, size_t size)
{
  size_t i;
  char c;

  if (address < EEPROM_START)
  {
    error(F("cannot save value to lower eeprom memory address"));
    return false;
  }

  for (i=0; i<size; i++)
  {
    c = *(src+i);
    eeprom_write_byte((uint8_t*)(address + i), (uint8_t)c);
    if (c == '\0')
    {
      break;
    }
  }

  return true;
}
