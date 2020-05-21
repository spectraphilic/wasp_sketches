#include "Coroutines.h"


/*
 * Logging functions. Very much like the print functions, they take formatted
 * strings, either as regular strings (char*) or stored in Flash (F).
 *
 * These rely on vlog for actual printing, which by default just prints to USB,
 * but vlog can be overriden.
 */

void Loop::log(loglevel_t level, const __FlashStringHelper *message)
{
  if (level <= loglevel)
  {
    // Copy to working memory
    char buffer[150];
    strncpy_F(buffer, message, sizeof(buffer));

    // Out
    vlog(level, buffer);
  }
}

void Loop::logf_P(loglevel_t level, PGM_P format, ...)
{
  if (level <= loglevel)
  {
    // Copy to working memory
    char buffer[150];
    strncpy_P(buffer, format, sizeof(buffer));

    // Format string
    char message[150];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), buffer, args);
    va_end(args);

    // Out
    vlog(level, message);
  }
}

void Loop::logf_(loglevel_t level, const char *format, ...)
{
  if (level <= loglevel)
  {
    // Format string
    char message[150];
    va_list args;
    va_start(args, format);
    vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    // Out
    vlog(level, message);
  }
}

const char* Loop::loglevel2str(loglevel_t level)
{
  switch (level)
  {
    case LOG_FATAL:
      return "FATAL";
    case LOG_ERROR:
      return "ERROR";
    case LOG_WARN:
      return "WARN";
    case LOG_INFO:
      return "INFO";
    case LOG_DEBUG:
      return "DEBUG";
    case LOG_TRACE:
      return "TRACE";
  }

  return "";
}

/*
 * Free functions that can be redefined (because declared weak).
 */

void vlog(loglevel_t level, const char* message)
{
  size_t size = 150;
  char buffer[size];

  // Timestamp + Level
  sprintf(buffer, "%lu %s ", millis(), cr.loglevel2str(level));
  // + Message
  size_t len = strlen(buffer);
  strncat(buffer, message, size - len - 1);

  // Out
  USB.println(buffer);
}

