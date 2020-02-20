#include <WaspSD.h>
#include <Coroutines.h>


/* Utils */
int sd_mkfile(const char*);
int sd_mkdir(const char*);
int sd_open(const char* filename, SdFile &file, uint8_t mode);
int sd_write(SdFile &file, const void* buf, size_t nbyte);
int sd_append(SdFile &file, const void* buf, size_t nbyte);

#define QUEUE_OK 0
#define QUEUE_ERROR 1
#define QUEUE_INDEX_ERROR 2

/*
 * Last-In-First-Out queue (LIFO)
 *
 * TODO Merge LIFO and FIFO into a new Queue, it will use a single file. This
 * file will have a header, including:
 *
 * - marker, to identify the file is a queue
 * - version number (1 byte)
 * - offset to the 1st element (4 bytes), or item-number (multiply by item-size)
 * - and, maybe, the item-size (1 byte)
 * - and maybe some reserved space
 *
 * This will simplify the code, save program memory, and make it easier to
 * change/upgrade.
 *
 */

class LIFO
{
  protected:
    // Input parameters
    const char* qname; // Filename of the queue file
    uint8_t item_size; // Size of records in the queue

    // State
    uint32_t queue_size;
    uint32_t offset = 0;
    int32_t nitems; // Signed because idx can be negative
    virtual int read_offset();
    int read_state();
    uint8_t _mode = 0;

    // Open files
    SdFile queue;

  public:
    // Constructor
    LIFO(const char* _queue, uint8_t _size) : qname(_queue), item_size(_size) {}

    virtual int make();       // Create files if not done already
    virtual int open(uint8_t mode);
    virtual void close();
    virtual int sync();
    int drop_end(uint8_t n);  // Remove the last n items from the queue
    int peek(uint8_t *, int32_t); // Return the item in the given position
    int push(uint8_t *);
    int32_t len() { return nitems; }
};


/*
 * First-In-First-Out queue (FIFO)
 */

class FIFO : public LIFO
{
  private:
    // Index
    const char* iname;
    SdFile index;
    int read_offset();

  public:
    // Constructor
    FIFO(const char* _queue, const char* _index, uint8_t _size) :
      LIFO(_queue, _size), iname(_index) {}

    int make();
    int open(uint8_t mode);
    void close();
    int sync();
    int drop_begin(uint8_t n); // Remove the first n items from the queue
};
