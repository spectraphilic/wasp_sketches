#include "Queue.h"


int FIFO::make()
{
  uint32_t start = 0;

  // Queue file
  if (sd_mkfile(qname)) { return QUEUE_ERROR; }

  // Index file
  if (sd_mkfile(iname)) { return QUEUE_ERROR; }
  if (SD.getFileSize(iname) == 0)
  {
    SD.openFile((char*)iname, &index, O_WRITE);
    sd_write(index, (void*)(&start), 4);
    index.close();
  }
}


int FIFO::open(uint8_t mode)
{
  if (sd_open(iname, index, mode))
  {
    return QUEUE_ERROR;
  }
  if (sd_open(qname, queue, mode))
  {
    index.close();
    return QUEUE_ERROR;
  }

  return QUEUE_OK;
}


int FIFO::close()
{
  if (index.isOpen()) { index.close(); }
  if (queue.isOpen()) { queue.close(); }
  return QUEUE_OK;
}

int FIFO::read_state()
{
  queue_size = queue.fileSize();
  uint8_t item[4];
  if (index.read(item, 4) != 4)
  {
    return QUEUE_ERROR;
  }
  offset = *(uint32_t *)item;

  return (offset < queue_size)? QUEUE_OK: QUEUE_EMPTY;
}

int FIFO::drop()
{
  if (open(O_RDWR)) { return QUEUE_ERROR; }

  // Read offset
  int status = read_state();
  if (status)
  {
    close();
    return status;
  }

  // Truncate queue to zero
  offset += item_size;
  if (offset >= queue_size)
  {
    offset = 0;
    if (queue.truncate(0) == false)
    {
      close();
      return QUEUE_ERROR;
    }
  }

  // Update offset
  index.seekSet(0);
  if (sd_write(index, (void*)(&offset), 4))
  {
    close();
    return QUEUE_ERROR;
  }

  close();
  return QUEUE_OK;
}


int FIFO::peek(uint8_t *item)
{
  if (open(O_READ)) { return QUEUE_ERROR; }

  // Read offset
  int status = read_state();
  if (status)
  {
    close();
    return status;
  }

  // Read the record
  queue.seekSet(offset);
  if (queue.read(item, item_size) != item_size)
  {
    close();
    return QUEUE_ERROR;
  }

  close();
  return QUEUE_OK;
}


int FIFO::push(uint8_t *item)
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

  queue.close();
  return QUEUE_OK;
}
