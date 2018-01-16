#include "Coroutines.h"


/**
 * Copy (strncpy_F) or concatenate (strncat_F) string from Flash to RAM.
 *
 * Parameters:
 * - dst        Pointer to the char array where the content is to be copied
 * - src        Pointer to the string to be copied from Flash memory
 * - size       Size of the destination array
 *
 * Return pointer to the destination string.
 *
 * This is like strncpy, but source is const __FlashStringHelper *
 */

char* strncpy_F(char* dst, const __FlashStringHelper * src, size_t size)
{
  const char * __attribute__((progmem)) p = (const char * ) src;
  unsigned char c;
  uint8_t i = 0;
  uint8_t n = size - 1;

  // XXX Is it possible to use strncpy_P instead ??
  do
  {
    c = pgm_read_byte(p++);
    dst[i] = c;
    i++;
  }
  while (c != 0 && i < n);
  dst[i] = '\0';

  return dst;
}

char* strncat_F(char* dst, const __FlashStringHelper * src, size_t size)
{
  size_t len = strlen(dst);

  strncpy_F(dst + len, src, size - len);
  return dst;
}

/**
 * Helper function to implement something like Python's '..'.join([])
 *
 * Concatenates src to dst, prepended with delimiter if dst is not empty; src
 * may be a formatted string.
 *
 * Usage example:
 *
 * buffer[0] = '\0';
 * strnjoin_F(buffer, sizeof(buffer), F(", "), F("A"));
 * strnjoin_F(buffer, sizeof(buffer), F(", "), F("B"));
 * strnjoin_F(buffer, sizeof(buffer), F(", "), F("C"));
 *
 * At the end buffer will hold "A, B, C"
 */
char* strnjoin_F(char* dst, size_t size, const __FlashStringHelper* delimiter, const __FlashStringHelper* src, ...)
{
  va_list args;
  const char * __attribute__((progmem)) p = (const char * ) src;
  char aux[100];

  if (dst[0])
  {
    strncat_F(dst, delimiter, size);
  }

  strncpy_F(aux, src, sizeof(aux));

  size_t len = strlen(dst);
  va_start(args, src);
  vsnprintf(dst + len, size - len - 1, aux, args);
  va_end(args);

  return dst;
}

/**
 * Version of snprintf that takes an F string.
 *
 */

int vsnprintf_F(char* dst, size_t size, const __FlashStringHelper* format, va_list args)
{
  char aux[size];

  strncpy_F(aux, format, size);
  return vsnprintf(dst, size, aux, args);
}

int snprintf_F(char* dst, size_t size, const __FlashStringHelper* format, ...)
{
  va_list args;
  int n;

  va_start(args, format);
  n = vsnprintf_F(dst, size, format, args);
  va_end(args);

  return n;
}
