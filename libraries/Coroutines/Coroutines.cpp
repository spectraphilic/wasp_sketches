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
    //log_trace("Cannot spawn, given delay too big");
    return -1;
  }

  if (next - first >= CR_QUEUE_SIZE)
  {
    //log_trace("Cannot spawn, limit reached");
    return -1;
  }

  if (next > CR_MAX_TID)
  {
    //log_trace("Cannot spawn, task ids exhausted");
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
    //log_trace("Cannot join older or same task");
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
    //log_trace("Task %d error", tid);
    return;
  }

  // Stop (Success)
  if (state == CR_TASK_STOP)
  {
    task->fun = NULL;
    //log_trace("Task %d done", tid);
    return;
  }

  // Suspend: delay
  if (state >= CR_DELAY_OFFSET)
  {
    task->state = state;
    //log_trace("Task %d suspended (delay = %lu)", tid, state);
    return;
  }

  // Suspend: join
  join(task, tid, (tid_t) state);
  //log_trace("Task %d suspended (join %lu)", tid);
  return;
}


void Loop::run()
{
  int32_t task_wait;
  Task* task;

  start = millis();

  while (next - first)
  {
    int32_t delay_time = 0;

    //USB.printf((char*)"%d %d\n", first, next);
    uint32_t now = millis() - start + CR_DELAY_OFFSET; // Relative now (since start), plus offset
    for (uint16_t tid=first; tid < next; tid++)
    {
      task = get(tid);
      if (task != NULL)
      {
        tstate_t state = task->state;

        // Waiting for some time to pass
        if (state >= CR_DELAY_OFFSET)
        {
          task_wait = state - now;
          if (task_wait <= 0)
          {
            //log_trace("Task %d time threshold reached: run", tid);
            resume(task, tid);
          }
          else
          {
            if (delay_time == 0 || task_wait < delay_time)
            {
              delay_time = task_wait;
            }
          }
          continue;
        }

        // Waiting for some other task to end
        if (state >= CR_MIN_TID)
        {
          if (get(state) == NULL)
          {
            //log_trace("Task %d finished: resume task %d", state, tid);
            resume(task, tid);
          }
        }
      }
    }
  }
}


/*
 * Utilities
 */

/*
 * Functions to calculate the distance between two calls to millis(), taking
 * into account overflow.
 *
 * XXX We could just remove this function and use millis() - t0 everywhere, but
 * it would use more program memory, which we want to avoid, at least for now.
 * That's why we declare the function noinline in the header file.
 */

uint32_t Loop::millisDiff(uint32_t t0)
{
  return millis() - t0;
}

bool Loop::timeout(uint32_t t0, uint32_t timeout)
{
  return (millis() - t0) > timeout;
}

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

/*
 * Instance
 */
Loop cr = Loop();
