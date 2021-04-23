#include "Queue.h"


int sd_mkfile(const char* name)
{
    SdFile file;

    if (! SD.openFile((char*)name, &file, O_RDWR | O_CREAT))
        return 1;

    file.close();
    return 0;
}


int sd_mkdir(const char* name)
{
    switch (SD.isDir(name)) {
        case 0: // file
            return 1;
        case -1: // error
            if (SD.mkdir((char*)name) == false)
                return 1;
    }
    return 0;
}


int sd_open(const char* filename, SdFile &file, uint8_t mode)
{
    // Do nothing if already open
    if (file.isOpen())
        return 0;

    // Open
    if (SD.openFile((char*)filename, &file, mode) == 0)
        return 1;

    return 0;
}


/**
 * Append data to the given file.
 *
 * Return 0 on success, or an error code.
 */
int sd_write(SdFile &file, const void* buf, size_t size)
{
    int n = file.write(buf, size);
    if (n < 0 || (size_t)n < size)
        return 1;

    return 0;
}


/**
 * Append data to the given file.
 *
 * Return 0 on success, or an error code.
 */
int sd_append(SdFile &file, const void* buf, size_t size)
{
    if (file.seekEnd() == false)
        return 1;

    return sd_write(file, buf, size);
}
