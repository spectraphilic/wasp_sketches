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

typedef unsigned int tid_t;
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
    _state=__LINE__; return (cr.millisDiff(cr.start) + delay + CR_DELAY_OFFSET); case __LINE__:; \
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

enum loglevel_t {OFF, FATAL, ERROR, WARN, INFO, DEBUG, TRACE};

#define fatal(fmt, ...) cr.log(FATAL, fmt, ## __VA_ARGS__)
#define error(fmt, ...) cr.log(ERROR, fmt, ## __VA_ARGS__)
#define warn(fmt, ...) cr.log(WARN, fmt, ## __VA_ARGS__)
#define info(fmt, ...) cr.log(INFO, fmt, ## __VA_ARGS__)
#define debug(fmt, ...) cr.log(DEBUG, fmt, ## __VA_ARGS__)
#define trace(fmt, ...) cr.log(TRACE, fmt, ## __VA_ARGS__)

/* By default prints to USB. Redefine this function to change behaviour. */
extern void vlog(loglevel_t level, const char* message, va_list args) __attribute__((weak));


class Loop
{
  private:
    Task queue[CR_QUEUE_SIZE]; // Task queue
    int16_t first, next; // Id of the first and next task id in the queue.


    // Private task functions
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
    uint32_t millisDiff(uint32_t t0, uint32_t t1);
    uint32_t millisDiff(uint32_t t0);

    // Printing to USB done right. The print functions takes a formatted
    // string, either a regular string (char*) or one stored in the program
    // memory (F). It appends a new line.
    void vprint(const char* message, va_list args);
    void print(const __FlashStringHelper *, ...);
    void print();

    // Logging
    loglevel_t loglevel = DEBUG;
    void log(loglevel_t level, const __FlashStringHelper *, ...);
    const char* loglevel2str(loglevel_t level);
};

// Utility functions to work with program memory (flash)
char* strncpy_F(char* dst, const __FlashStringHelper * src, size_t num);
char* strncat_F(char* dst, const __FlashStringHelper * src, size_t size);
char* strnjoin_F(char* dst, size_t size, const __FlashStringHelper* delimiter, const __FlashStringHelper* src, ...);

extern Loop cr;



#endif
