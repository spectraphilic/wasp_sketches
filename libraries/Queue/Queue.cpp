#include "Queue.h"

int Queue::push(uint8_t *item)
{
  if (sd_open(qname, queue, O_RDWR | O_CREAT))
  {
    return QUEUE_ERROR;
  }

  // Security check, the file size must be a multiple of 8. If it is not we
  // consider there has been a write error, and we trunctate the file.
  queue_size = queue.fileSize();
  //cr.println(F("** push(): %lu"), queue_size);
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

  queue.close();
  return QUEUE_OK;
}
