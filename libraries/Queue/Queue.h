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
class FIFO
{
  private:
  public:
    const char* qname; // Filename of the queue file
    const char* iname; // Filename of the index file
    uint8_t size;      // Size of records in the queue

    SdFile queue;      // Queue file
    SdFile index;      // Index file

    // Constructor
    FIFO(const char* _queue, const char* _index, uint8_t _size) :
      qname(_queue), iname(_index), size(_size) {}

    int make(); // Creates and initializes files if they don't exist already
    int open();
    int close();
    int drop(); // Removes one item from the queue

    //push();
    //peek();
    //pop();
    //drop();
};
