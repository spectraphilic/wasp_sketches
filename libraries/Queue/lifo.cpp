#include "Queue.h"


int LIFO::make()
{
  // Queue file
  if (sd_mkfile(qname)) { return QUEUE_ERROR; }
}


int LIFO::open(uint8_t mode)
{
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

int LIFO::drop()
{
  if (open(O_RDWR)) { return QUEUE_ERROR; }

  // Read offset
  int status = read_state();
  if (status)
  {
    close();
    return status;
  }
  //cr.println(F("** lifo.drop(): %lu %lu"), offset, queue_size);

  // Truncate (pop)
  if (queue.truncate(queue_size - item_size) == false)
  {
    close();
    return QUEUE_ERROR;
  }

  close();
  return QUEUE_OK;
}


int LIFO::peek(uint8_t *item)
{
  if (open(O_READ)) { return QUEUE_ERROR; }

  // Read state
  int status = read_state();
  if (status)
  {
    close();
    return status;
  }
  //cr.println(F("** lifo.peek(): %lu %lu"), offset, queue_size);

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
