#include "WaspUIO.h"


/*
 * Print file contents to usb
 */
COMMAND(cmdCat)
{
  char filename[80];

  // Check feature availability
  if (! UIO.hasSD) { return cmd_unavailable; }

  // Check input
  if (sscanf(str, "%79s", filename) != 1) { return cmd_bad_input; }
  if (strlen(filename) == 0) { return cmd_bad_input; }

  // Do
  SD.showFile((char*) filename);
  return cmd_quiet;
}

/*
 * Print file contents of binary file to usb
 */
COMMAND(cmdCatx)
{
  char filename[80];
  SdFile file;
  uint32_t size;
  uint8_t idx;
  int chr;

  // Check feature availability
  if (! UIO.hasSD) { return cmd_unavailable; }

  // Check input
  if (sscanf(str, "%79s", filename) != 1) { return cmd_bad_input; }
  if (strlen(filename) == 0) { return cmd_bad_input; }

  // Open file
  if (! SD.openFile(filename, &file, O_READ)) { return cmd_error; }

  size = file.fileSize();
  for (idx=0; idx < size; idx++)
  {
    chr = file.read();
    if (chr < 0)
    {
      file.close();
      return cmd_error;
    }
    USB.printHex((char)chr);
    USB.print(" ");
  }

  file.close();
  USB.println();

  return cmd_quiet;
}

/*
 * Soft format, just remove all files and dirs.
 */
bool format_file_cb(SdBaseFile &parent, char * name)
{
  return SdBaseFile::remove(&parent, name);
}

bool format_after_cb(SdBaseFile &me, char * name)
{
  return me.isRoot() || me.rmdir();
}

COMMAND(cmdFormat)
{
  // Check feature availability
  if (! UIO.hasSD) { return cmd_unavailable; }

  // Soft format
  UIO.walk(SD.root, NULL, format_file_cb, format_after_cb);

  // Create base files
  UIO.stopSD(); UIO.startSD();

  return cmd_ok;
}

/*
 * List files in SD.
 */
COMMAND(cmdLs)
{
  // Check feature availability
  if (! UIO.hasSD) { return cmd_unavailable; }

  // Do
  SD.ls(LS_DATE | LS_SIZE | LS_R);
  return cmd_quiet;
}

/*
 * Remove file
 * TODO Try rewrite using walk to see whether we can save some memory.
 */
bool ls_before_cb(SdBaseFile &parent, char * name)
{
  cr.println(F("%s/"), name);
  return true;
}

bool ls_file_cb(SdBaseFile &me, char * name)
{
  cr.println(F("%s"), name);
  return true;
}

COMMAND(cmdRm)
{
  char filename[80];

  // Check feature availability
  if (! UIO.hasSD) { return cmd_unavailable; }

  // Check input
  if (sscanf(str, "%79s", filename) != 1) { return cmd_bad_input; }
  if (strlen(filename) == 0) { return cmd_bad_input; }

  // Do
  SD.del(filename);
  return cmd_ok;
}

/*
 * Print Tail lines of FILENAME to USB
 */
COMMAND(cmdTail)
{
  unsigned int maxnl, nl;
  char filename[80];
  SdFile file;
  uint32_t size, nc;
  uint16_t max = DOS_BUFFER_SIZE - 1;
  int16_t c;
  size_t nbyte;
  int nread;

  // Check feature availability
  if (! UIO.hasSD) { return cmd_unavailable; }

  // Check input
  if (sscanf(str, "%u %79s", &maxnl, &filename) != 2) { return cmd_bad_input; }
  if (strlen(filename) == 0) { return cmd_bad_input; }

  // Open file
  if (!SD.openFile(filename, &file, O_READ)) { return cmd_error; }

  // Read backwards
  maxnl++;
  size = file.fileSize();
  nc = 0;
  nl = 0;
  while (nc < size && nl < maxnl)
  {
    nc++;
    file.seekEnd(-nc);
    if ((c = file.read()) < 0) { goto error; }
    if (c == '\n') { nl++; }
  }
  nc--; // do not include the last newline read

  // Read forward
  cr.println(F("-------------------------"));
  file.seekEnd(-nc);
  while (nc > 0)
  {
    nbyte = (nc < max) ? nc : max;
    nread = file.read(SD.buffer, nbyte);
    if (nread < 0) { goto error; }
    USB.print(SD.buffer);
    nc -= nread;
  }
  cr.println(F("-------------------------"));

  file.close();
  return cmd_quiet;

error:
  file.close();
  return cmd_error;
}
