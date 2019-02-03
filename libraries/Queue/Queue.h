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

class Queue
{
  protected:
    // Input parameters
    const char* qname; // Filename of the queue file
    uint8_t item_size; // Size of records in the queue

    // State
    uint32_t queue_size;
    virtual int read_state() = 0;

    // Open files
    SdFile queue;

  public:
    // Constructor
    Queue(const char* _queue, uint8_t _size) : qname(_queue), item_size(_size) {}

    virtual int make() = 0;          // Create files if not done already
    virtual int open(uint8_t mode) = 0;
    virtual int close() = 0;
    virtual int drop() = 0;          // Remove one item from the queue
    virtual int peek(uint8_t *) = 0; // Return one item from the queue
    int push(uint8_t *);
};


/*
 * Last-In-First-Out queue (LIFO)
 */

class LIFO : public Queue
{
  private:
    int read_state();

  public:
    LIFO(const char* _queue, uint8_t _size) : Queue(_queue, _size) {}

    int make();
    int open(uint8_t mode);
    int close();
    int drop();
    int peek(uint8_t *);
};


/*
 * First-In-First-Out queue (FIFO)
 */

class FIFO : public Queue
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
      Queue(_queue, _size), iname(_index) {}

    int make();
    int open(uint8_t mode);
    int close();
    int drop();
    int peek(uint8_t *);
};
