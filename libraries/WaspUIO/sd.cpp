#include "WaspUIO.h"


/**
 * Functions to start and stop SD. Open and closes required files.
 */

void WaspUIO::startSD()
{
    if (WaspRegister & REG_SD)
        return;

    // This fixs a bug with J firmware, see
    // https://github.com/spectraphilic/wasp_sketches/issues/52
    SD.OFF();
    hasSD = SD.ON(); // ~ 270ms

    if (hasSD) {
        baselayout();
    } else {
        cr_printf("ERROR SD.ON() flag=%u %d\n", SD.flag, SD.card.errorCode());
    }
}

void WaspUIO::stopSD()
{
    if (SPI.isSD) {
        // There's already a delay(100) in SD.OFF()
        // With this extra dealy the SD will have 150ms to handle pending operations
        delay(50);

        // Off
        SD.OFF();
    }
}


/*
 * Create basic filesystem layout. To be called just after format and when
 * booting, to be sure the filesystem is good.
 */
int WaspUIO::baselayout()
{
    int error = 0;

    if (sd_mkdir(archive_dir)) // Data directory
        error = 1;

    if (sd_mkfile(logFilename)) // Log file
        error = 1;

#if WITH_IRIDIUM
    if (fifo.make()) // Queue (first-in-first-out)
        error = 1;
    if (lifo.make()) // Queue (last-in-first-out)
        error = 1;
#elif WITH_4G
    if (lifo.make()) // Queue (last-in-first-out)
        error = 1;
#else
    if (fifo.make()) // Queue (first-in-first-out)
        error = 1;
#endif

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
          log_warn("before_cb failed %s", name);
          return -1;
        }
        err = walk(subdir, before_cb, file_cb, after_cb);
        if (err)
        {
          return err;
        }
        if (after_cb != NULL && ! after_cb(subdir, name))
        {
          log_warn("after_cb failed %s", name);
          return -1;
        }
      }
      else
      {
        log_debug("Error opening %s", name);
        return -1;
      }
    }
    else
    {
      if (file_cb != NULL && ! file_cb(root, name))
      {
        log_warn("file_cb failed %s", name);
        return -1;
      }
    }

    root.seekSet(pos);
    err = root.readDir(&dir_entry); // Next
  }

  return err;
}
