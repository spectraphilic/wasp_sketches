/*
 * The functions override weak functions defined elsewhere (in Coroutines/
 * actually). Define behaviour specific to the project.
 */

#include "WaspUIO.h"


/**
 * Function to log waspmote activity.
 *
 * Parameters:
 * - log level: FATAL, ERROR, WARNING, INFO, DEBUG, TRACE
 * - formatted message: F string
 * - arguments for the formatted string
 *
 * Returns: bool             - true on success, false for error
 *
 * The output message max length is 149 chars (not counting null char).
 */

void vlog(loglevel_t level, const char* message)
{
    size_t size = 150;
    char buffer[size];
    size_t max = size - 1;
    size_t len;
    unsigned long seconds;
    uint16_t ms;

    // (1) Prepare message
    // Timestamp
    seconds = UIO.getEpochTime(ms);
    cr_sprintf(buffer, "%lu.%03u ", seconds, ms);
    len = strlen(buffer);
    // Level
    cr_sprintf(buffer + len, "%s ", cr.loglevel2str(level));
    len = strlen(buffer);
    // Message
    strncat(buffer, message, size - len - 1);
    len = strlen(buffer);
    // Newline
    if (len == max)
        len--; // Avoid buffer overflow
    buffer[len] = '\n';
    buffer[len + 1] = '\0';

    // (2) Print to USB
    if (UIO.log_usb) {
        USB.ON();
        USB.flush(); // XXX This fixes a weird bug with XBee
        USB.print(buffer);
        USB.OFF();
    }

    // (3) Print to log file
    if (UIO.hasSD && UIO.log_sd) {
        if (sd_open(UIO.logFilename, UIO.logFile, O_WRITE | O_CREAT | O_APPEND | O_SYNC)) {
            cr_printf("Open log file failed\n");
            return;
        }

        if (sd_append(UIO.logFile, buffer, strlen(buffer))) {
            cr_printf("Append to log file failed\n");
            UIO.logFile.close();
        }
    }
}
