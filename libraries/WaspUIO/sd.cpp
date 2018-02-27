#include "WaspUIO.h"


int WaspUIO::createFile(const char* name)
{
  SdFile file;

  if (! SD.openFile((char*)name, &file, O_RDWR | O_CREAT))
  {
    return 1;
  }

  file.close();
  return 0;
}


int WaspUIO::createDir(const char* name)
{
  switch (SD.isDir(name))
  {
    case 0: // file
      return 1;
    case -1: // error
      if (SD.mkdir((char*)name) == false) { return 1; }
  }
  return 0;
}


int WaspUIO::openFile(const char* filename, SdFile &file, uint8_t mode)
{
  if (! file.isOpen())
  {
    if (SD.openFile((char*)filename, &file, mode) == 0)
    {
      cr.set_last_error(F("openFile(%s) failed"), filename);
      return 1;
    }
  }

  return 0;
}

/*
 * Create basic filesystem layout. To be called just after format and when
 * booting, to be sure the filesystem is good.
 */
int WaspUIO::baselayout()
{
  int error = 0;

  if (createDir(archive_dir))  { error = 1; } // Data directory
  if (createFile(logFilename)) { error = 1; } // Log file
  if (createFile(tmpFilename)) { error = 1; } // Tmp file (index to frames)

  return error;
}


/**
 * Append data to the given file.
 *
 * Return 0 on success, 1 on error.
 */
int WaspUIO::append(SdFile &file, const void* buf, size_t size)
{
  int n;

  if (file.seekEnd() == false)
  {
    cr.set_last_error(F("append seekEnd failed"));
    return 1;
  }

  n = file.write(buf, size);
  if (n == -1)
  {
    cr.set_last_error(F("append write failed"));
    return 1;
  }

  if (file.sync() == false)
  {
    cr.set_last_error(F("append sync failed"));
    return 1;
  }

  if (n < size)
  {
    cr.set_last_error(F("append wrote only %d bytes of %u"), n, size);
    return 1;
  }

  return 0;
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
