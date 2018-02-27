#include "Coroutines.h"


void Loop::set_last_error(const __FlashStringHelper * format, ...)
{
  va_list args;

  va_start(args, format);
  vsnprintf_F(last_error, sizeof(last_error), format, args);
  va_end(args);
}
