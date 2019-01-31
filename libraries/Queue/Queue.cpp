#include "Queue.h"


int createFile(const char* name)
{
  SdFile file;

  if (! SD.openFile((char*)name, &file, O_RDWR | O_CREAT))
  {
    return 1;
  }

  file.close();
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


int FIFO::touch()
{
  uint32_t start = 0;

  // Queue file
  if (createFile(qname)) { return 1; }

  // Index file
  if (createFile(iname)) { return 1; }
  if (SD.getFileSize(iname) == 0)
  {
    SD.openFile((char*)iname, &index, O_WRITE);
    sd_write(index, (void*)(&start), 4);
    index.close();
  }
}
