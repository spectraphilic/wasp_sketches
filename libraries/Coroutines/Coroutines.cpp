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
  uint32_t now; // Relative now (since start), plus offset
  int32_t task_wait;
  int32_t delay_time;
  bool resumed;
  Task* task;

  start = millis();
  sleep_time = 0;

  while (next - first)
  {
    delay_time = 0;
    resumed = false;

    //USB.printf((char*)"%d %d\n", first, next);
    now = millis() - start + sleep_time + CR_DELAY_OFFSET;
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
            //trace(F("Task %d time threshold reached: run"), tid);
            resume(task, tid);
            resumed = true;
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
            //trace(F("Task %d finished: resume task %d"), state, tid);
            resume(task, tid);
            resumed = true;
          }
        }
      }
    }

    // Sleep
    if (! resumed && delay_time > 0)
    {
      //sleep(delay_time);
    }
  }
}


void Loop::sleep(int32_t delay_time)
{
  uint32_t start, sleep, now;
  int32_t left;
  uint8_t seconds;
  char alarmTime[12]; // "00:00:00:00"
  uint8_t timer;

  const uint16_t overhead_head = 200; // mostly turning off the SD
  const uint16_t overhead_body = 300;
  const uint16_t overhead_tail = 300; // mostly turning on the SD
  const uint16_t overhead_min = overhead_head + overhead_body + overhead_tail;
  const uint16_t min_sleep = 128;
  uint32_t until = delay_time - overhead_tail - overhead_body;

  if (delay_time < (overhead_min + min_sleep))
  {
    return;
  }

  start = millis();
  sleep = 0;

  beforeSleep(); // ~ 200 ms
  PWR.clearInterruptionPin();
  intFlag = 0;
  while (true)
  {
    //uint32_t t0 = millis();
    now = (millis() - start) + sleep;
    left = until - now;

    // Stop condition
    if (left < min_sleep)
    {
      break;
    }

    // If less than 1s, use the internal watchdog (overhead ~247ms)
    if (left < 1000)
    {
      // XXX Documentation says 250ms..8s but 256ms..8192ms makes more sense to me
      if      (left > 500) { timer = WTD_500MS; sleep += 500; }
      else if (left > 250) { timer = WTD_250MS; sleep += 250; }
      else if (left > 128) { timer = WTD_128MS; sleep += 128; }

      // Sleep
      PWR.sleep(timer, ALL_ON);
      PWR.setWatchdog(WTD_OFF, timer); // Awake: Disable watchdog
      if (intFlag & WTD_INT)           // Clear flag
      {
        intFlag &= ~(WTD_INT);
      }
    }

    // If more than 1s, use the RTC (overhead ~281ms)
    else
    {
      seconds = left / 1000;
      if (seconds > 59)
      {
        seconds = 59;
      }
      sleep += (seconds * 1000);

      // Sleep
      sprintf(alarmTime, "00:00:00:%02d", seconds);
      PWR.deepSleep(alarmTime, RTC_OFFSET, RTC_ALM1_MODE5, ALL_ON);
      if (intFlag & RTC_INT) // Clear flag
      {
        intFlag &= ~(RTC_INT); RTC.alarmTriggered = 0;
      }
      RTC.attachInt(); // To keep the reset watchdog (RTC alarm 2)
    }

    // Catch unexpected interruptions
    if (intFlag)
    {
      println(F("Unexpected interruption %d"), intFlag);
      intFlag = 0;
    }
    //println(F("sleep %lu overhead=%lu"), left, millis() - t0);
  }
  afterSleep(); // ~282ms
  sleep_time += sleep;

  //println(F("cpu=%lu sleep=%lu delay=%d"), millis() - start, sleep, delay_time);
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

void Loop::log(loglevel_t level, const __FlashStringHelper * format, ...)
{
  if (level <= loglevel)
  {
    char message[150];
    strncpy_F(message, format, sizeof(message));

    va_list args;
    va_start(args, format);
    vlog(level, message, args);
    va_end(args);
  }
}

void Loop::log(loglevel_t level, const char * format, ...)
{
  if (level <= loglevel)
  {
    va_list args;
    va_start(args, format);
    vlog(level, format, args);
    va_end(args);
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

void vlog(loglevel_t level, const char* message, va_list args)
{
  size_t size = 150;
  char buffer[size];

  // Timestamp + Level + Message
  sprintf(buffer, "%lu %s ", millis(), cr.loglevel2str(level));
  size_t len = strlen(buffer);
  vsnprintf(buffer + len, size - len - 1, message, args);
  USB.println(buffer);
}

void beforeSleep() {}
void afterSleep() {}

/*
 * Instance
 */
Loop cr = Loop();
