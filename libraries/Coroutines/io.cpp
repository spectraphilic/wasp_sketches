#include "Coroutines.h"


/**
 * Ask the user for input through the USB cable
 *
 * Parameters:
 * - buffer  : output buffer where the data read will be stored
 * - size    : size of the buffer
 * - timeout : Number of seconds to wait for input, or 0 to wait forever (max 255)
 *
 * Returns:
 * - String read from the USB cable, or NULL if timeout.
 */

const char* Loop::input(char* buffer, size_t size, unsigned long timeout)
{
  int i = 0;

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


/*
 * Functions to print formatted strings to usb. Take only F strings (stored in
 * Flash).
 */

void Loop::print(const __FlashStringHelper * format, ...)
{
  va_list args;
  char out[120];

  va_start(args, format);
  vsnprintf_F(out, sizeof(out), format, args);
  va_end(args);

  USB.print(out);
}

void Loop::println(const __FlashStringHelper * format, ...)
{
  va_list args;
  char out[120];

  va_start(args, format);
  vsnprintf_F(out, sizeof(out), format, args);
  va_end(args);

  USB.println(out);
}

void Loop::println()
{
  USB.println();
}
