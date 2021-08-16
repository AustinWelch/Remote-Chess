#ifndef G8RTOS_SCHEDULER_H_
#define G8RTOS_SCHEDULER_H_

#include "G8RTOS_Structures.h"
// #include "core_cm4.h"
#include <stdint.h>

#define MAX_THREADS 5
#define MAX_PTHREADS 2
#define STACKSIZE 1500
#define OSINT_PRIORITY 7


typedef enum {
      G8RTOS_SUCCESS
    , G8RTOS_TOO_MANY_THREADS
    , G8RTOS_INCORRECTLY_ALIVE
    , G8RTOS_CANNOT_KILL_LAST_THREAD
    , G8RTOS_THREAD_DOES_NOT_EXIST
    , G8RTOS_INVALID_IRQN
    , G8RTOS_INVALID_PRIORITY
} SchedulerStatus;

typedef struct {
	SchedulerStatus status;
	ThreadID_t id;
} NewThreadStatus;

/* Holds the current time for the whole System */
extern uint_fast32_t systemTime;

void G8RTOS_Init(); // Initialize the OS and prepare to have threads added

// Initializes the systick scheduler and starts the first thread.
// This function will only return iff starting the scheduler fails.
int32_t G8RTOS_Launch(); 


// Adds a thread to the OS and sets up its execution context.
NewThreadStatus G8RTOS_AddThread(ThreadEntry_g entryFunc, uint8_t priority, const char name[MAX_TCB_NAME_LENGTH]);

// Adds a periodic thread to the OS. It will have no execution context.
SchedulerStatus G8RTOS_AddPeriodicThread(ThreadEntry_g handler, uint_fast32_t period, uint_fast32_t firstStartTime);

// Add an aperiodic thread to the OS. It has no execution context.
// Priority cannot be greater than 6
SchedulerStatus G8RTOS_AddAperiodicThread(ThreadEntry_g handler, uint8_t priority, int_fast32_t irqn);

// Kills the given thread if it exists. Cannot kill the last thread remaining
SchedulerStatus G8RTOS_KillThread(ThreadID_t id);

// Kills the currently running thread
SchedulerStatus G8RTOS_KillSelf();

void G8RTOS_YieldThread(); // Yield the current thread
void G8RTOS_SleepThread(uint_fast32_t ms); // Sleep for minimum of `ms` number of milliseconds

ThreadID_t G8RTOS_GetCurrentThreadID(void);

// Request to disallow context switches
void G8RTOS_SetThreadSwitchable(uint8_t canSwitch);

// Return whether the current execution context is in a periodic thread
bool_t G8RTOS_InPThread();

#endif /* G8RTOS_SCHEDULER_H_ */
