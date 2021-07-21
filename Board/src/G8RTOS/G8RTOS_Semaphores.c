/*
 * G8RTOS_Semaphores.c
 */

/*********************************************** Dependencies and Externs *************************************************************/

// #include <stdint.h>
#include "msp.h"
#include "G8RTOS_Semaphores.h"
#include "G8RTOS_CriticalSection.h"
#include "G8RTOS_Scheduler.h"
#include "G8RTOS_Structures.h"

extern TCB* currentThread;
extern TCBLL runningThreads;
extern TCBLL blockedThreads;

/*********************************************** Public Functions *********************************************************************/

/*
 * Initializes a semaphore to a given value
 * Param "s": Pointer to semaphore
 * Param "value": Value to initialize semaphore to
 * THIS IS A CRITICAL SECTION
 */
// void G8RTOS_InitSemaphore(semaphore_t *s, int32_t value)
// {
// 	/* Implement this */
// }

/*
 * Waits for a semaphore to be available (value greater than 0)
 * 	- Decrements semaphore when available
 * 	- Spinlocks to wait for semaphore
 * Param "s": Pointer to semaphore to wait on
 * THIS IS A CRITICAL SECTION
 */
void G8RTOS_AcquireSemaphore(Semaphore* s) {
	//while (1) {
		G8RTOS_StartCriticalSection();

		if (s->count != 0) {
			s->count--;
			G8RTOS_EndCriticalSection();

			return;
		}

		// Thread is blocked, yield control to OS
		currentThread->blockedSem = s;

		// Remove thread from running list, add to blocked list
		// Assumes not the last running thread
		tcbll_Remove(&runningThreads, currentThread);
		tcbll_Insert(&blockedThreads, currentThread);

		G8RTOS_EndCriticalSection();
		G8RTOS_YieldThread();
		s->count--;
	//}
}

/*
 * Signals the completion of the usage of a semaphore
 * 	- Increments the semaphore value by 1
 * Param "s": Pointer to semaphore to be signalled
 * THIS IS A CRITICAL SECTION
 */
void G8RTOS_ReleaseSemaphore(Semaphore* s) {
	G8RTOS_StartCriticalSection();
	s->count++;

	// Find the next blocked thread by this semaphore and unblock it
	TCB* thr = blockedThreads.head;

	if (thr != NULL) {
		do {
			if (thr->blockedSem == s) {
				thr->blockedSem = NULL;

				// Remove from blocked threads, add to running threads list
				tcbll_Remove(&blockedThreads, thr);
				tcbll_Insert(&runningThreads, thr);

				break;
			}

			thr = thr->next;
		} while (thr != blockedThreads.head);
	}

	G8RTOS_EndCriticalSection();
}

bool_t G8RTOS_IsSemaphoreAvailable(const Semaphore* s) {
	return s->count > 0;
}

