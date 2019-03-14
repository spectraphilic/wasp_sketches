#include <WaspSD.h>
#include <Coroutines.h>


/* Utils */
int sd_mkfile(const char*);
int sd_mkdir(const char*);
int sd_open(const char* filename, SdFile &file, uint8_t mode);
int sd_write(SdFile &file, const void* buf, size_t nbyte);
int sd_append(SdFile &file, const void* buf, size_t nbyte);


/*
 * Base stuff for queues
 */

#define QUEUE_OK 0
#define QUEUE_EMPTY 1
#define QUEUE_ERROR 2

/*
 * Last-In-First-Out queue (LIFO)
 */

class LIFO
{
  protected:
    // Input parameters
    const char* qname; // Filename of the queue file
    uint8_t item_size; // Size of records in the queue

    // State
    uint32_t queue_size;
    int read_state();

    // Open files
    SdFile queue;

  public:
    // Constructor
    LIFO(const char* _queue, uint8_t _size) : qname(_queue), item_size(_size) {}

    int make();               // Create files if not done already
    int open(uint8_t mode);
    int close();
    int drop_last();          // Remove the last item from the queue
    int peek_last(uint8_t *); // Return the last item from the queue

    int push(uint8_t *);
};


/*
 * First-In-First-Out queue (FIFO)
 */

class FIFO : public LIFO
{
  private:
    int read_state();
    // Index
    const char* iname;
    uint32_t offset;
    SdFile index;

  public:
    // Constructor
    FIFO(const char* _queue, const char* _index, uint8_t _size) :
      LIFO(_queue, _size), iname(_index) {}

    int make();
    int open(uint8_t mode);
    int close();
    int drop_first();
    int peek_first(uint8_t *);
};
