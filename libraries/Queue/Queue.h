#include <WaspSD.h>

/*
 * Some global functions
 */

int sd_mkfile(const char*);
int sd_mkdir(const char*);
int sd_open(const char* filename, SdFile &file, uint8_t mode);
int sd_write(SdFile &file, const void* buf, size_t nbyte);
int sd_append(SdFile &file, const void* buf, size_t nbyte);

/*
 * First-In-First-Out queue (FIFO)
 */

#define QUEUE_OK 0
#define QUEUE_EMPTY 1
#define QUEUE_ERROR 2

class FIFO
{
  private:
    // Input parameters
    const char* qname; // Filename of the queue file
    const char* iname; // Filename of the index file
    uint8_t item_size; // Size of records in the queue

    // State
    uint32_t queue_size;
    uint32_t offset;
    int read(); // Reads offset and queue size

    // Open files
    SdFile queue;
    SdFile index;

  public:
    // Constructor
    FIFO(const char* _queue, const char* _index, uint8_t _size) :
      qname(_queue), iname(_index), item_size(_size) {}

    int make(); // Create and initialize files if they don't exist already
    int open(uint8_t mode);
    int close();
    int drop();          // Remove one item from the queue
    int peek(uint8_t *); // Return one item from the queue
    int push(uint8_t *);
};
