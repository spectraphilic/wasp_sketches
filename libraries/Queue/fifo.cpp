#include "Queue.h"


int FIFO::make()
{
  uint32_t start = 0;

  // Queue file
  if (sd_mkfile(qname)) { return 1; }

  // Index file
  if (sd_mkfile(iname)) { return 1; }
  if (SD.getFileSize(iname) == 0)
  {
    sd_open(iname, index, O_WRITE | O_SYNC);
    sd_write(index, (void*)(&start), 4);
    index.close();
  }

  return 0;
}


int FIFO::open(uint8_t mode)
{
  // If already open, do nothing. Then it's the responsability of the caller to
  // open with the right mode.
  if (_mode != 0) { return 0; }
  //cr_printf("FIFO::open(%s, %d -> %d)\n", qname, _mode, mode);

  // Open
  if (sd_open(qname, queue, mode))
  {
    return 1;
  }
  if (sd_open(iname, index, mode))
  {
    queue.close();
    return 1;
  }
  _mode = mode;

  // Read state
  if (read_state())
  {
    close();
    return 1;
  }

  // Check for file corruption. The file size must be a multiple of item_size.
  // If it is not we consider there has been a write error, and we truncate the
  // file.
  if (mode && O_WRITE)
  {
    uint32_t mod = queue_size % item_size;
    if (mod != 0)
    {
      queue.truncate(queue_size - mod);
    }
  }

  return 0;
}


void FIFO::close()
{
  //cr_printf("FIFO::close(%s)\n", qname);
  if (queue.isOpen()) { queue.close(); }
  if (index.isOpen()) { index.close(); }
  _mode = 0;
}

int FIFO::sync()
{
  if (queue.sync() == false) { return 1; } // Error
  if (index.sync() == false) { return 1; } // Error
  return 0;
}

int FIFO::read_offset()
{
  uint8_t item[4];

  index.seekSet(0);
  if (index.read(item, 4) != 4)
  {
    return 1;
  }
  offset = *(uint32_t *)item;
  return 0;
}


int FIFO::drop_begin(uint8_t n)
{
  // Open
  bool closed = (_mode == 0);
  if (open(O_RDWR | O_SYNC)) { return 1; }

  // Truncate queue to zero
  offset += item_size * n;
  if (offset >= queue_size) {
    offset = 0;
    if (queue.truncate(0) == false)
    {
      close();
      return 1;
    }
    nitems = 0;
  } else {
    nitems -= n;
  }

  // Update offset
  index.seekSet(0);
  if (sd_write(index, (void*)(&offset), 4))
  {
    close();
    return 1;
  }

  // Close
  if (closed) { close(); } // Close only if it was closed before
  return 0;
}
