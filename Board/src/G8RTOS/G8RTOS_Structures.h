#ifndef G8RTOS_STRUCTURES_H_
#define G8RTOS_STRUCTURES_H_

#include "G8RTOS_Semaphores.h"
#include "G8RTOS_Defs.h"
#include <stdint.h>

#define MAX_TCB_NAME_LENGTH 16

typedef uint_fast32_t ThreadID_t;

typedef struct detail_TCB {
    int32_t* stackPtr;
    struct detail_TCB* previous;
    struct detail_TCB* next;
    Semaphore* blockedSem;
    uint_fast32_t sleepTime; // Time to sleep the thread
    uint8_t priority; // Lower numerical value means higher priority
    bool_t alive; // Whether this thread block contains a thread in any state (active, blocked, sleeping, etc.)
    ThreadID_t id;
    char name[MAX_TCB_NAME_LENGTH];
} TCB;

typedef struct detail_PTCB {
    ThreadEntry_g handler;
    uint_fast32_t executionTime; // System time of the next execution
    uint_fast32_t period;        // How long to wait in between executions
} PTCB;

typedef struct detail_TCBLL {
    TCB* head;
    uint8_t size;
} TCBLL;

// Inserts a TCB into the end of the TCB linked list
// Assumes the list is circular
void tcbll_Insert(TCBLL* list, TCB* toInsert);

// Removes a TCB from the TCB linked list it is in
// Assumes the list is circular
void tcbll_Remove(TCBLL* list, TCB* toRemove);

#endif /* G8RTOS_STRUCTURES_H_ */
