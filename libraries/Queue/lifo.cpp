#include "Queue.h"


int LIFO::make()
{
  // Queue file
  if (sd_mkfile(qname)) { return QUEUE_ERROR; }
}


int LIFO::open(uint8_t mode)
{
  // TODO Create if doesn't exist
  if (sd_open(qname, queue, mode))
  {
    return QUEUE_ERROR;
  }

  return QUEUE_OK;
}


int LIFO::close()
{
  if (queue.isOpen()) { queue.close(); }
  return QUEUE_OK;
}

int LIFO::read_state()
{
  queue_size = queue.fileSize();
  return (queue_size > 0)? QUEUE_OK: QUEUE_EMPTY;
}

int LIFO::push(uint8_t *item)
{
  if (sd_open(qname, queue, O_RDWR | O_CREAT))
  {
    return QUEUE_ERROR;
  }

  // Security check, the file size must be a multiple of 8. If it is not we
  // consider there has been a write error, and we trunctate the file.
  queue_size = queue.fileSize();
  uint32_t mod = queue_size % item_size;
  if (mod != 0)
  {
    queue.truncate(queue_size - mod);
  }

  // Append record
  if (sd_append(queue, item, item_size))
  {
    queue.close();
    return QUEUE_ERROR;
  }

  //cr.println(F("** push(): %lu"), queue.fileSize());
  queue.close();
  return QUEUE_OK;
}

int LIFO::drop_last()
{
  if (open(O_RDWR)) { return QUEUE_ERROR; }

  // Read offset
  int status = read_state();
  if (status)
  {
    close();
    return status;
  }
  //cr.println(F("** lifo.drop_last(): %lu"), queue_size);

  // Truncate (pop)
  if (queue.truncate(queue_size - item_size) == false)
  {
    close();
    return QUEUE_ERROR;
  }

  close();
  return QUEUE_OK;
}


int LIFO::peek_last(uint8_t *item)
{
  if (open(O_READ)) { return QUEUE_ERROR; }

  // Read state
  int status = read_state();
  if (status)
  {
    close();
    return status;
  }
  //cr.println(F("** lifo.peek_last(): %lu"), queue_size);

  // Read the record
  queue.seekEnd(-item_size);
  if (queue.read(item, item_size) != item_size)
  {
    close();
    return QUEUE_ERROR;
  }

  close();
  return QUEUE_OK;
}
