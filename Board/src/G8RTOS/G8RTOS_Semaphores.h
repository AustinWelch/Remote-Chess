/*
 * G8RTOS_Semaphores.h
 */

#ifndef G8RTOS_SEMAPHORES_H_
#define G8RTOS_SEMAPHORES_H_

#include <stdint.h>
#include "G8RTOS_Defs.h"

typedef struct {
    uint_fast8_t count;
} Semaphore;

/*
 * Initializes a semaphore to a given value
 * Param "s": Pointer to semaphore
 * Param "value": Value to initialize semaphore to
 */
// void G8RTOS_InitSemaphore(Semaphore* s, int32_t value);

/*
 * Waits for a semaphore to be available (value greater than 0)
 * 	- Decrements semaphore when available
 * 	- Spinlocks to wait for semaphore
 * Param "s": Pointer to semaphore to wait on
 */
void G8RTOS_AcquireSemaphore(Semaphore* sem);

/*
 * Signals the completion of the usage of a semaphore
 * 	- Increments the semaphore value by 1
 * Param "s": Pointer to semaphore to be signalled
 */
void G8RTOS_ReleaseSemaphore(Semaphore* sem);

bool_t G8RTOS_IsSemaphoreAvailable(const Semaphore* sem);

#endif /* G8RTOS_SEMAPHORES_H_ */
