#include "WaspUIO.h"


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
