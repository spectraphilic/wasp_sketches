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
  uint32_t start = 0;

  if (createDir(archive_dir))  { error = 1; } // Data directory
  if (createFile(logFilename)) { error = 1; } // Log file
  if (createFile(queueFilename)) { error = 1; } // Queue file
  if (createFile(qstartFilename)) { error = 1; } // Queue file

  if (SD.getFileSize(qstartFilename) == 0)
  {
    SD.openFile((char*)qstartFilename, &qstartFile, O_WRITE);
    write(qstartFile, (void*)(&start), 4);
    qstartFile.close();
  }

  return error;
}


/**
 * Append data to the given file.
 *
 * Return 0 on success, 1 on error.
 */
int WaspUIO::write(SdFile &file, const void* buf, size_t size)
{
  int n;

  n = file.write(buf, size);
  if (n == -1)
  {
    cr.set_last_error(F("write failed"));
    return 1;
  }

  if (file.sync() == false)
  {
    cr.set_last_error(F("write sync failed"));
    return 1;
  }

  if (n < size)
  {
    cr.set_last_error(F("write wrote only %d bytes of %u"), n, size);
    return 1;
  }

  return 0;
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

  return write(file, buf, size);
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


/*
 * Walk filesystem tree in pre-order
 */
void dir_cb(dir_t & dir_entry)
{
  char dir_name[13];
  SdBaseFile::dirName(dir_entry, dir_name);
  cr.println(F("%s/"), dir_name);
}

void file_cb(dir_t & dir_entry)
{
  char dir_name[13];
  SdBaseFile::dirName(dir_entry, dir_name);
  cr.println(F("%s"), dir_name);
}


int8_t WaspUIO::walk(SdBaseFile &root)
{
  int8_t err;
  dir_t dir_entry;
  char dir_name[13];
  SdBaseFile subdir;

  root.rewind();
  err = root.readDir(&dir_entry);
  while (err > 0)
  {
    SdBaseFile::dirName(dir_entry, dir_name);
    if (dir_entry.attributes && DIR_ATT_DIRECTORY)
    {
      dir_cb(dir_entry);
      uint32_t pos = root.curPosition();
      if (subdir.open(&root, dir_name, O_READ))
      {
        err = walk(subdir);
        if (err)
        {
          return err;
        }
      }
      else
      {
        debug(F("Error opening %s"), dir_name);
        return -1;
      }
      root.seekSet(pos);
    }
    else
    {
      file_cb(dir_entry);
    }

    err = root.readDir(&dir_entry); // Next
  }

  return err;
}
