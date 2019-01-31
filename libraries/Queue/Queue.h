#include <WaspSD.h>

/*
 * Some global functions
 */

int createFile(const char*);
int sd_write(SdFile &file, const void* buf, size_t nbyte);
int sd_append(SdFile &file, const void* buf, size_t nbyte);

/*
 * First-In-First-Out queue (FIFO)
 */
class FIFO
{
  private:
    const char* qname; // Filename of the queue file
    const char* iname; // Filename of the index file
    uint8_t size;      // Size of records in the queue

    SdFile queue;      // Queue file
    SdFile index;      // Index file

  public:
    // Constructor
    FIFO(const char* _queue, const char* _index, uint8_t _size) :
      qname(_queue), iname(_index), size(_size) {}

    // Creates and initializes files if they don't exist already
    int touch();

    //push();
    //peek();
    //pop();
    //drop();
};
