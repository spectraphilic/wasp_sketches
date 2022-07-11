#include "WaspUIO.h"
#include <SDI12.h>


WaspSDI12 sdi(PIN_SDI12);

const char* WaspUIO::sdi_command(const char *cmd)
{
    size_t size = 116 + 1;
    char buffer[size];

    // rstrip
    const char *end = cmd + strlen(cmd) - 1;
    while (end >= cmd && isspace(*end))
        end--;

    // lstrip
    const char *begin = cmd;
    while (begin <= end && isspace(*begin))
        begin++;

    // Empty
    if (*begin == '\0' || isspace(*begin))
        return NULL;

    // Copy and finish command string
    size_t n = end - begin + 1;
    if (n > size - 2)
        n = size - 2;
    memcpy(buffer, begin, n);
    buffer[n] = '!';
    buffer[n+1] = '\0';

    // Send command
    return sdi.sendCommand(buffer);
}
