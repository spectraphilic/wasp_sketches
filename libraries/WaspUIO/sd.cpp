#include "WaspUIO.h"


/**
 * Functions to start and stop SD. Open and closes required files.
 */

void WaspUIO::startSD()
{
  if (WaspRegister & REG_SD)
    return;

  hasSD = SD.ON();
  if (! hasSD)
  {
    cr.println(F("SD.ON() failed flag=%u"), SD.flag);
    return;
  }

  baselayout();
}

void WaspUIO::stopSD()
{
  if (SPI.isSD)
  {
    // Close files
    if (logFile.isOpen()) { logFile.close(); }
    if (queueFile.isOpen()) { queueFile.close(); }
    if (qstartFile.isOpen()) { qstartFile.close(); }
    // Off
    SD.OFF();
  }
}


/*
 * Create/Open
 */

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
  if (fifo.touch()) { error = 1; }            // Fifo files

  return error;
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
int8_t WaspUIO::walk(SdBaseFile &root,
                     bool (*before_cb)(SdBaseFile &me, char* name),
                     bool (*file_cb)(SdBaseFile &parent, char* name),
                     bool (*after_cb)(SdBaseFile &me, char* name))
{
  int8_t err;
  dir_t dir_entry;
  char name[13];
  SdBaseFile subdir;

  root.rewind();
  err = root.readDir(&dir_entry);
  while (err > 0)
  {
    SdBaseFile::dirName(dir_entry, name);
    uint32_t pos = root.curPosition();

    if (dir_entry.attributes && DIR_ATT_DIRECTORY)
    {
      if (subdir.open(&root, name, O_READ))
      {
        if (before_cb != NULL && ! before_cb(subdir, name))
        {
          warn(F("before_cb failed %s"), name);
          return -1;
        }
        err = walk(subdir, before_cb, file_cb, after_cb);
        if (err)
        {
          return err;
        }
        if (after_cb != NULL && ! after_cb(subdir, name))
        {
          warn(F("after_cb failed %s"), name);
          return -1;
        }
      }
      else
      {
        debug(F("Error opening %s"), name);
        return -1;
      }
    }
    else
    {
      if (file_cb != NULL && ! file_cb(root, name))
      {
        warn(F("file_cb failed %s"), name);
        return -1;
      }
    }

    root.seekSet(pos);
    err = root.readDir(&dir_entry); // Next
  }

  return err;
}
