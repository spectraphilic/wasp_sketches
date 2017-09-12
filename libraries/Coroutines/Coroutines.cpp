#include "Coroutines.h"


void Loop::reset()
{
  first = CR_MIN_TID;
  next = CR_MIN_TID;
  start = 0;
}

int Loop::spawn(tstate_t (*fun)(), unsigned int delay)
{
  if (delay > CR_MAX_DELAY)
  {
    //trace(F("Cannot spawn, given delay too big"));
    return -1;
  }

  if (next - first >= CR_QUEUE_SIZE)
  {
    //trace(F("Cannot spawn, limit reached"));
    return -1;
  }

  if (next > CR_MAX_TID)
  {
    //trace(F("Cannot spawn, task ids exhausted"));
    return -1;
  }

  Task& task = queue[next % CR_QUEUE_SIZE];
  if (start)
  {
    delay += millisDiff(start);
  }
  task.state = delay + CR_DELAY_OFFSET;
  task.fun = fun;

  return next++;
}

Task* Loop::get(tid_t tid)
{
  uint8_t idx;

  if (tid < first || tid >= next)
  {
    return NULL;
  }

  Task& task = queue[tid % CR_QUEUE_SIZE];
  if (task.fun == NULL)
  {
    // Garbage collect dead tasks only from the beginning.
    // Do not collect from the end to avoid reusing tids.
    if (tid == first)
    {
      first++;
    }
    return NULL;
  }

  return &task;
}

int8_t Loop::join(Task* task, tid_t tid, tid_t target_tid)
{
  // Simple system to prevent dead locks
  if (tid >= target_tid)
  {
    //trace(F("Cannot join older or same task"));
    return -1;
  }

  // Join
  task->state = target_tid;
  return 0;
}


void Loop::resume(Task* task, tid_t tid)
{
  // Run
  tstate_t state = task->fun();

  // Error
  if (state == CR_TASK_ERROR)
  {
    task->fun = NULL;
    //trace(F("Task %d error"), tid);
    return;
  }

  // Stop (Success)
  if (state == CR_TASK_STOP)
  {
    task->fun = NULL;
    //trace(F("Task %d done"), tid);
    return;
  }

  // Suspend: delay
  if (state >= CR_DELAY_OFFSET)
  {
    task->state = state;
    //trace(F("Task %d suspended (delay = %lu)"), tid, state);
    return;
  }

  // Suspend: join
  join(task, tid, (tid_t) state);
  //trace(F("Task %d suspended (join %lu)"), tid);
  return;
}


void Loop::run()
{
  start = millis();
  Task* task;

  // Reset if stuck for 4 minutes (v15 only)
  if (_boot_version >= 'H')
  {
    RTC.setWatchdog(4);
  }

  while (next - first)
  {
    //USB.printf((char*)"%d %d\n", first, next);
    uint32_t time_threshold = millisDiff(start) + CR_DELAY_OFFSET;
    for (uint16_t tid=first; tid < next; tid++)
    {
      task = get(tid);
      if (task != NULL)
      {
        tstate_t state = task->state;

        // Wait for some time
        if (state >= CR_DELAY_OFFSET)
        {
          if (state <= time_threshold)
          {
            //trace(F("Task %d time threshold reached: run"), tid);
            resume(task, tid);
          }
          continue;
        }

        // Waiting for some other task to end
        if (state >= CR_MIN_TID)
        {
          if (get(state) == NULL)
          {
            //trace(F("Task %d finished: resume task %d"), state, tid);
            resume(task, tid);
          }
        }
      }
    }
  }

  // v15 only
  if (_boot_version >= 'H')
  {
    RTC.unSetWatchdog();
  }
}


/*
 * Utilities
 */

/**
 * Copy (strncpy_F) or concatenate (strncat_F) string from Flash to RAM.
 *
 * Parameters:
 * - dst        Pointer to the char array where the content is to be copied
 * - src        Pointer to the string to be copied from Flash memory
 * - size       Size of the destination array
 *
 * Return pointer to the destination string.
 *
 * This is like strncpy, but source is const __FlashStringHelper *
 */

char* strncpy_F(char* dst, const __FlashStringHelper * src, size_t size)
{
  const char * __attribute__((progmem)) p = (const char * ) src;
  unsigned char c;
  uint8_t i = 0;
  uint8_t n = size - 1;

  // XXX Is it possible to use strncpy_P instead ??
  do
  {
    c = pgm_read_byte(p++);
    dst[i] = c;
    i++;
  }
  while (c != 0 && i < n);
  dst[i] = '\0';

  return dst;
}

char* strncat_F(char* dst, const __FlashStringHelper * src, size_t size)
{
  size_t len = strlen(dst);

  strncpy_F(dst + len, src, size - len);
  return dst;
}

/**
 * Helper function to implement something like Python's '..'.join([])
 *
 * Concatenates src to dst, prepended with delimiter if dst is not empty; src
 * may be a formatted string.
 *
 * Usage example:
 *
 * buffer[0] = '\0';
 * strnjoin_F(buffer, sizeof(buffer), F(", "), F("A"));
 * strnjoin_F(buffer, sizeof(buffer), F(", "), F("B"));
 * strnjoin_F(buffer, sizeof(buffer), F(", "), F("C"));
 *
 * At the end buffer will hold "A, B, C"
 */
char* strnjoin_F(char* dst, size_t size, const __FlashStringHelper* delimiter, const __FlashStringHelper* src, ...)
{
  va_list args;
  const char * __attribute__((progmem)) p = (const char * ) src;
  char aux[100];

  if (dst[0])
  {
    strncat_F(dst, delimiter, size);
  }

  strncpy_F(aux, src, sizeof(aux));

  size_t len = strlen(dst);
  va_start(args, src);
  vsnprintf(dst + len, size - len - 1, aux, args);
  va_end(args);

  return dst;
}


/*
 * Functions to calculate the distance between two calls to millis(), taking
 * into account overflow.
 */

uint32_t Loop::millisDiff(uint32_t t0, uint32_t t1)
{
  return (t1 >= t0) ? t1 - t0: (ULONG_MAX - t0) + t1;
}

uint32_t Loop::millisDiff(uint32_t t0)
{
    return millisDiff(t0, millis());
}

/*
 * Functions to print formatted strings to usb. Can take regular strings
 * (char*) or those stored in Flash (F).
 */

void Loop::vprint(const char* message, va_list args)
{
  char buffer[128];
  size_t len, max;
  uint32_t seconds;
  uint16_t ms;

  vsnprintf(buffer, sizeof(buffer), message, args);
  USB.println(buffer);
}

void Loop::print(const __FlashStringHelper * ifsh, ...)
{
  va_list args;
  char message[100];

  strncpy_F(message, ifsh, sizeof(message));

  va_start(args, ifsh);
  vprint(message, args);
  va_end(args);
}

void Loop::print()
{
  USB.println();
}

/*
 * Logging functions. Very much like the print functions, they take formatted
 * strings, either as regular strings (char*) or stored in Flash (F).
 *
 * These rely on vlog for actual printing, which by default just prints to USB,
 * but vlog can be overriden.
 */

void vlog(loglevel_t level, const char* message, va_list args)
{
  cr.vprint(message, args);
}

void Loop::log(loglevel_t level, const __FlashStringHelper * ifsh, ...)
{
  if (level > loglevel)
  {
    return;
  }

  va_list args;
  char message[120];

  strncpy_F(message, ifsh, sizeof(message));

  va_start(args, ifsh);
  vlog(level, message, args);
  va_end(args);
}

const char* Loop::loglevel2str(loglevel_t level)
{
  switch (level)
  {
    case FATAL:
      return "FATAL";
    case ERROR:
      return "ERROR";
    case WARN:
      return "WARN";
    case INFO:
      return "INFO";
    case DEBUG:
      return "DEBUG";
    case TRACE:
      return "TRACE";
  }

  return "";
}

// Instance
Loop cr = Loop();
