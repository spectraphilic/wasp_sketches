/*
 * Coroutines.
 *
 * Non-preemptive multitasking library inspired by coroutines:
 *
 * - Tasks can be suspendend/resumed, including from inner functions.
 * - Suspended tasks can be waiting for: (1) some time to pass or (2) some
 *   other task to complete.
 *
 * Based on https://www.chiark.greenend.org.uk/~sgtatham/coroutines.html but
 * with the ability to suspend/resume from inner functions.
 *
 * The stack is not preserved after resuming, just the entry point with the
 * help of an static variable. This means tasks are not reentrant, cannot spawn
 * the same task twice at the same time.
 *
 * This has been designed to run on MCUs with low memory, hence only the
 * minimum features needed have been implemented to save memory.
 *
 * Writing a task involves the use of some macros. Example:
 *
 *   CR_TASK(task1)
 *   {
 *     CR_BEGIN;
 *
 *     [...]
 *     CR_DELAY(200);  // Suspends the task, will be resumed after 200ms
 *     [...]
 *
 *     CR_SPAWN(task2); // Spawns another task
 *
 *     CR_END;
 *   }
 *
 *   cr.reset();
 *   cr.spawn(&task1);
 *   cr.run();
 *
 * Example to spawn a task then wait for it to complete (spawn/join):
 *
 *   CR_TASK(task1)
 *   {
 *     static tid task2_id;
 *
 *     CR_BEGIN;
 *
 *     CR_SPAWN2(task2, task2_id); // Spawns another task
 *     CR_JOIN(task2_id); // Suspend, will be resumed when task2 completes
 *
 *     CR_END;
 *   }
 *
 * The main object 'cr' includes some other utilities and features, most
 * notably logging.
 *
 */

#ifndef COROUTINES_H
#define COROUTINES_H

#include <inttypes.h>
#include <limits.h> // ULONG_MAX
#include <WaspClasses.h>

#ifndef CR_QUEUE_SIZE
#define CR_QUEUE_SIZE 20
#endif


/*
 * Types and macros related to the multitasking system
 */

typedef uint16_t tid_t;
typedef uint32_t tstate_t;

#define CR_TASK(name) tstate_t name(void)
#define CR_TASK_STOP 0
#define CR_TASK_ERROR 1
#define CR_MIN_TID 2
#define CR_MAX_TID 10000
#define CR_DELAY_OFFSET CR_MAX_TID + 1
#define CR_MAX_DELAY ULONG_MAX - CR_MAX_TID

#define CR_BEGIN     static int _state = 0; switch(_state) { case 0:;
#define CR_DELAY(delay) \
  do { \
    _state=__LINE__; return (millis() - cr.start + delay + CR_DELAY_OFFSET); case __LINE__:; \
  } while (0)

#define CR_ERROR  do { _state=0; return CR_TASK_ERROR; } while(0)
#define CR_RETURN do { _state=0; return CR_TASK_STOP; } while (0)
#define CR_END    } _state=0; return CR_TASK_STOP
#define CR_WAIT(fun) \
  do { \
    _state=__LINE__; case __LINE__:;  \
    tstate_t _tstate = fun(); \
    if      (_tstate == CR_TASK_ERROR) { _state=0; return CR_TASK_ERROR; } \
    else if (_tstate != CR_TASK_STOP)  { return _tstate; } \
  } while(0)

#define CR_SPAWN(fun) cr.spawn(fun, 0)
#define CR_SPAWN2(fun, tid) tid = cr.spawn(fun, 0);
#define CR_JOIN(tid) \
  if (cr.get(tid) != NULL) { _state=__LINE__; return tid; case __LINE__:; }


typedef struct Task
{
  tstate_t (*fun)();
  tstate_t state;
} Task;


/*
 * Types and macros related to the logging system
 */

enum loglevel_t {
  LOG_OFF,
  LOG_FATAL,
  LOG_ERROR,
  LOG_WARN,
  LOG_INFO,
  LOG_DEBUG,
  LOG_TRACE,
  LOG_LEN // Special value
};

#ifdef WFORMAT

#define cr_printf(fmt, ...) cr.printf(fmt, ## __VA_ARGS__)
#define cr_snprintf(s, n, fmt, ...) snprintf(s, n, fmt, ## __VA_ARGS__)
#define cr_sprintf(s, fmt, ...) sprintf(s, fmt, ## __VA_ARGS__)
#define log_fatal(fmt, ...) cr.log(LOG_FATAL, fmt, ## __VA_ARGS__)
#define log_error(fmt, ...) cr.log(LOG_ERROR, fmt, ## __VA_ARGS__)
#define log_warn(fmt, ...) cr.log(LOG_WARN, fmt, ## __VA_ARGS__)
#define log_info(fmt, ...) cr.log(LOG_INFO, fmt, ## __VA_ARGS__)
#define log_debug(fmt, ...) cr.log(LOG_DEBUG, fmt, ## __VA_ARGS__)
#define log_trace(fmt, ...) cr.log(LOG_TRACE, fmt, ## __VA_ARGS__)

#else

#define cr_printf(fmt, ...) cr.printf_P(PSTR(fmt), ## __VA_ARGS__)
#define cr_snprintf(s, n, fmt, ...) snprintf_P(s, n, PSTR(fmt), ## __VA_ARGS__)
#define cr_sprintf(s, fmt, ...) sprintf_P(s, PSTR(fmt), ## __VA_ARGS__)
#define log_fatal(fmt, ...) cr.log_P(LOG_FATAL, PSTR(fmt), ## __VA_ARGS__)
#define log_error(fmt, ...) cr.log_P(LOG_ERROR, PSTR(fmt), ## __VA_ARGS__)
#define log_warn(fmt, ...) cr.log_P(LOG_WARN, PSTR(fmt), ## __VA_ARGS__)
#define log_info(fmt, ...) cr.log_P(LOG_INFO, PSTR(fmt), ## __VA_ARGS__)
#define log_debug(fmt, ...) cr.log_P(LOG_DEBUG, PSTR(fmt), ## __VA_ARGS__)
#define log_trace(fmt, ...) cr.log_P(LOG_TRACE, PSTR(fmt), ## __VA_ARGS__)

#endif


/*
 * Weak free functions to be overriden. By default they do nothing.
 * Override them to define behaviour.
 */

/* Logging */
extern void vlog(loglevel_t level, const char* message) __attribute__((weak));


class Loop
{
  private:
    Task queue[CR_QUEUE_SIZE]; // Task queue
    uint16_t first, next; // Id of the first and next task id in the queue.

    // Private functions
    int8_t join(Task* task, tid_t tid, tid_t target_id);
    void resume(Task* task, tid_t tid);

  public:
    // 'get' is public only because used by the macros. It is not really
    // intended to be used from the outside.
    Task* get(tid_t tid);

    // Public only because used by the macros
    // Initialized to zero when cr.reset() is called. Means loop not running.
    // Initialized to millis() when cr.run() is called.
    uint32_t start;

    // Main functions
    void reset();
    int spawn(tstate_t (*fun)(), unsigned int delay=0);
    void run();

    // millisDiff is to calculate the difference between two calls to millis(),
    // but taking into account overflow. This is only needed if the device runs
    // without sleep for longer than 49 days.
    uint32_t millisDiff(uint32_t t0) __attribute__((noinline));
    bool timeout(uint32_t t0, uint32_t timeout);

    // Printing to USB done right. The print functions takes a formatted
    // string, either a regular string (char*) or one stored in the program
    // memory (F).  The println variant append a new line.
    const char* input(char* buffer, size_t size, unsigned long timeout);
    void printf(const char *format, ...) __attribute__ ((format (printf, 2, 3)));
    void printf_P(PGM_P format, ...);

    // Logging
    loglevel_t loglevel = LOG_DEBUG;
    const char* loglevel2str(loglevel_t level);
    void log(loglevel_t level, const char *format, ...) __attribute__ ((format (printf, 3, 4)));
    void log_P(loglevel_t level, PGM_P format, ...);

};

extern Loop cr;


#endif
