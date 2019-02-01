#include "Queue.h"


int sd_mkfile(const char* name)
{
  SdFile file;

  if (! SD.openFile((char*)name, &file, O_RDWR | O_CREAT))
  {
    return 1;
  }

  file.close();
  return 0;
}


int sd_mkdir(const char* name)
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


int sd_open(const char* filename, SdFile &file, uint8_t mode)
{
  if (! file.isOpen())
  {
    if (SD.openFile((char*)filename, &file, mode) == 0)
    {
      return 1;
    }
  }

  return 0;
}


/**
 * Append data to the given file.
 *
 * Return 0 on success, or an error code.
 */
int sd_write(SdFile &file, const void* buf, size_t size)
{
  int n;

  n = file.write(buf, size);
  if (n == -1)
  {
    return 1; // write failed
  }

  if (file.sync() == false)
  {
    return 2; // sync failed
  }

  if (n < size)
  {
    return 3; // wrote only n bytes of size
  }

  return 0;
}


/**
 * Append data to the given file.
 *
 * Return 0 on success, or an error code.
 */
int sd_append(SdFile &file, const void* buf, size_t size)
{
  if (file.seekEnd() == false)
  {
    return 4; // seekEnd failed
  }

  return sd_write(file, buf, size);
}
