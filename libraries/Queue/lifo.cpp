#include "Queue.h"


int LIFO::make()
{
  // Queue file
  if (sd_mkfile(qname)) { return 1; }
}


int LIFO::open(uint8_t mode)
{
  // If already open, do nothing. Then it's the responsability of the caller to
  // open with the right mode.
  if (_mode != 0) { return 0; }
  //cr.println(F("LIFO::open(%s, %d -> %d)"), qname, _mode, mode);

  // Open
  if (sd_open(qname, queue, mode))
  {
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


void LIFO::close()
{
  //cr.println(F("LIFO::close(%s)"), qname);
  if (queue.isOpen()) { queue.close(); }
  _mode = 0;
}


int LIFO::sync()
{
  if (queue.sync() == false) { return 1; } // Error
  return 0;
}


int LIFO::read_offset()
{
  return 0;
}

int LIFO::read_state()
{
  queue_size = queue.fileSize();
  if (read_offset())       { return 1; }
  if (offset > queue_size) { return 1; }
  nitems = (queue_size - offset) / item_size;

  return 0;
}

int LIFO::push(uint8_t *item)
{
  // Open
  bool closed = (_mode == 0);
  if (open(O_RDWR | O_CREAT | O_SYNC)) { return 1; }

  // Append
  if (sd_append(queue, item, item_size))
  {
    close();
    return 1;
  }
  nitems++;

  // Close
  if (closed) { close(); } // Close only if it was closed before
  return 0;
}

int LIFO::drop_end(uint8_t n)
{
  // Open
  bool closed = (_mode == 0);
  if (open(O_RDWR | O_SYNC)) { return 1; }

  // Truncate (pop)
  if (queue.truncate(queue_size - item_size * n) == false)
  {
    close();
    return 1;
  }
  nitems -= n;

  // Close
  if (closed) { close(); } // Close only if it was closed before
  return 0;
}


int LIFO::peek(uint8_t *item, int32_t idx)
{
  // Open
  bool closed = (_mode == 0);
  if (open(O_READ)) { return 1; }

  // Indexing
  if (idx < 0) { idx = nitems + idx; } // Negative index: -1 (last) ..
  if (idx < 0 || idx+1 > nitems)       // Check index is within range
  {
    close();
    return 1;
  }

  // Read the record
  queue.seekSet(offset + idx * item_size);
  if (queue.read(item, item_size) != item_size)
  {
    close();
    return 1;
  }

  // Close
  if (closed) { close(); } // Close only if it was closed before
  return 0;
}
