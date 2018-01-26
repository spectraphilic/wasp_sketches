#include "WaspUIO.h"


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

  n = file.write(buf, nbyte);
  if (n == -1)
  {
    error(F("append(%s): write failed"), tmpFilename);
    return -1;
  }

  if (file.sync() == false)
  {
    error(F("append(%s): sync failed"), tmpFilename);
    return -1;
  }

  return n;
}


/**
 * Read a line from the given open file, not including the end-of-line
 * character. Store the read line in SD.buffer.
 *
 * Return the length of the line. Or -1 for EOF. Or -2 if error.
 */

int WaspUIO::readline(SdFile &file)
{
  int n;

  n = file.fgets(SD.buffer, sizeof(SD.buffer));

  // Error
  if (n == -1)
    return -2;

  // EOF
  if (n == 0)
    return -1;

  if (SD.buffer[n - 1] == '\n') {
    SD.buffer[n - 1] = '\0';
    return n - 1;
  }

  return n;
}
