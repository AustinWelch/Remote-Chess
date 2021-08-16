/*
 * G8RTOS_Scheduler.c
 */

/*********************************************** Dependencies and Externs *************************************************************/

#include <stdint.h>
#include "msp.h"
#include "cs.h"
#include "core_cm4.h"
#include "G8RTOS_Scheduler.h"
#include "G8RTOS_Structures.h"
#include "G8RTOS_Semaphores.h"
#include "G8RTOS_CriticalSection.h"
#include "inc/ClockSys.h"
#include <string.h>

/* Status Register with the Thumb-bit Set */
#define THUMB_BIT 0x01000000

extern void G8RTOS_Start(); // Defined in ASM
extern void G8RTOS_StartFirstThread(void); // Defined in ASM
extern uint_fast32_t SystemCoreClock; // System Core Clock From system_msp432p401r.c

extern TCB* currentThread; // Define currentThread globally so it is accessible from ASM
TCB* currentThread = 0;

static uint_fast32_t exceptionReturnValue;

static TCB threadControlBlocks[MAX_THREADS];
static int32_t threadStacks[MAX_THREADS][STACKSIZE];
static uint_fast32_t numberThreads;

// running and blockedThreads are not static so semaphores can access them
TCBLL runningThreads  = (TCBLL) { .head = NULL, .size = 0 };
TCBLL blockedThreads  = (TCBLL) { .head = NULL, .size = 0 };
static TCBLL sleepingThreads = (TCBLL) { .head = NULL, .size = 0 };

static PTCB pthreads[MAX_PTHREADS];
static uint8_t numberPThreads;

uint_fast32_t systemTime = 0; // Unit is in milliseconds

static bool_t canContextSwitch = TRUE;
static bool_t inPThread = FALSE;
static bool_t isYielding = FALSE;

uint_fast32_t G8RTOS_GetExcReturn() {
	return exceptionReturnValue;
}

static bool_t IsThreadActive(const TCB* thr) {
	return thr->alive && thr->sleepTime == 0 && thr->blockedSem == NULL;
}

static bool_t IsThreadSleeping(const TCB* thr) {
	return thr->sleepTime != 0;
}

static bool_t IsThreadBlocked(const TCB* thr) {
	return thr->blockedSem != NULL;
}

void G8RTOS_Scheduler() {
	// Assumes that there is an available thread. Does not check.

	TCB* searchStart = NULL;
	TCB* nextThr = NULL;
	TCB* thr = NULL;

	// First, check if the current thread is active. If it is, we can just get
	// the next in the list
	if (IsThreadActive(currentThread)) {
		searchStart = currentThread->next;
	} else {
		// If the current thread is not active, get the head of the running threads list
		// This can happen when the thread is blocked/slept and the subsequent context switch is triggered
		searchStart = runningThreads.head;
	}

	thr = searchStart->next;
	nextThr = searchStart;

	uint8_t numActiveThreads = runningThreads.size;

	// Subtract 1 because we don't need to check the searchStart
	for (uint8_t i = 0; i < (numActiveThreads - 1); i++) {
		// Lower numerical value means higher priorty
		// Use strictly less than so equal priority threads all get chance to run
		// (it uses the first highest priority thread it finds. This is OK as long as threads
		// remain in the same relative order. However, getting repeated sleeps/blocks can
		// change the order)

		// The first isYielding clause allows a lower priority to overtake a yielding thread
		if (   (thr->priority < nextThr->priority || (nextThr == currentThread && isYielding))
			&& (thr != currentThread && isYielding)) // This prevents a yielding thread to overtake
		{
			nextThr = thr;
		}

		thr = thr->next;
	}

	isYielding = FALSE; // If we were yielding before, we are done now.
	currentThread = nextThr;
}

void TriggerContextSwitch() {
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // Trigger SV Handler to context switch (found in ASM)
}

void G8RTOS_YieldThread() {
	if (IsThreadActive(currentThread)) // Check if the current thread is actively yielding and now yielding from block
		isYielding = TRUE;

	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // Trigger SV Handler to context switch (found in ASM)
}

void G8RTOS_SleepThread(uint_fast32_t ms) {
	G8RTOS_StartCriticalSection();
	currentThread->sleepTime = ms;

	// Assumes there is always another running thread
	tcbll_Remove(&runningThreads, currentThread); // Remove currentThread from the running threads list
	tcbll_Insert(&sleepingThreads, currentThread); // Insert current thread into the sleeping threads list

	G8RTOS_EndCriticalSection();
	TriggerContextSwitch();
}

/*
 * SysTick Handler
 * Currently the Systick Handler will only increment the system time
 * and set the PendSV flag to start the scheduler
 *
 * In the future, this function will also be responsible for sleeping threads and periodic threads
 */
void SysTick_Handler() {
	for (uint8_t i = 0; i < numberPThreads; i++) {
		PTCB* pthread = &pthreads[i];

		if (systemTime == pthread->executionTime) {
			pthread->executionTime += pthread->period; // Set the next time for this pthread to execute

			inPThread = TRUE;
			pthread->handler(); // Run the handler for this pthread
			inPThread = FALSE;

			break; // Assume that we will only execute one pthread per quantum
		}
	}

	// Count down sleeping threads and unsleep when ready
	if (sleepingThreads.size != 0) {
		TCB* thr = sleepingThreads.head;

		uint8_t numSleeping = sleepingThreads.size;

		for (uint8_t i = 0; i < numSleeping; i++) {
			thr->sleepTime--;

			if (thr->sleepTime == 0) {
				// Unsleep this thread

				TCB* nextThr = thr->next;

				tcbll_Remove(&sleepingThreads, thr); // Remove from sleeping threads
				tcbll_Insert(&runningThreads, thr); // Add to running threads

				thr = nextThr;
			} else {
				thr = thr->next;
			}
		}
	}

	if (canContextSwitch && numberThreads > 1)
		SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk; // Trigger SV Handler to context switch (found in ASM)

	systemTime++;
}


// Enables the SysTick and its Interrupt. The intterupt is responsible for the context switch. 
// numCycles: Number of cycles between each context switch
static void InitSysTick(uint_least32_t numCycles) {
	SysTick_Config(numCycles);

	SysTick_enableInterrupt();

	SCB->SHP[11] = 0x80; // Give the SysTick second-lowest priority, which is vector number 15, or 11 in the table
}

// Enables the SVCall handler and interrupt with low priority
static void InitSVCall() {
	// IRQ_SetPriority(11, 7); // 11 is the SVCall vector number
	SCB->SHP[7] = 0xe0; // SVCall is number 11, which is the 7th item in the SHP table. e0 is the lowest priority (least important)
	SCB->SHP[10] = 0xe0; // SVPend is number 14, which is the 10th item in the SHP table. e0 is the lowest priority (least important)
}

// Inits variables (SystemTime, # threads), fastest clock, disables watchdog
void G8RTOS_Init() {
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer

	// Copy all 57 vectors of the vector table to the SRAM
	// (necessary to use aperiodic events)
	uint_fast32_t newVTORTable = 0x20000000;
	memcpy((uint_fast32_t*) newVTORTable, (uint_fast32_t*) SCB->VTOR, 57*4);
	SCB->VTOR = newVTORTable; // Switch the vector table over

	systemTime = 0;
	numberThreads = 0;
	numberPThreads = 0;

	for (uint8_t i = 0; i < MAX_THREADS; i++) {
		threadControlBlocks[i].alive = FALSE;
	}
}


/*
 * Starts G8RTOS Scheduler
 * 	- Initializes the Systick
 * 	- Sets Context to first thread
 * Returns: Error Code for starting scheduler. This will only return if the scheduler fails
 */
int G8RTOS_Launch() {
	InitSVCall();

	uint_fast32_t cpuFreq = CS_getMCLK();
	InitSysTick(cpuFreq / 1000); // Div 1000 to tick every millisecond

	currentThread = &threadControlBlocks[0];

	// Get the highest priority thread to launch first
	for (uint8_t i = 0; i < numberThreads; i++) {
		if (threadControlBlocks[i].priority <  currentThread->priority) {
			currentThread = &threadControlBlocks[i];
		}
	}

	G8RTOS_StartFirstThread();

	return 0;
}



/*
 * Adds threads to G8RTOS Scheduler
 * 	- Checks if there are stil available threads to insert to scheduler
 * 	- Initializes the thread control block for the provided thread
 * 	- Initializes the stack for the provided thread to hold a "fake context"
 * 	- Sets stack tcb stack pointer to top of thread stack
 * 	- Sets up the next and previous tcb pointers in a round robin fashion
 * Param "threadToAdd": Void-Void Function to add as preemptable main thread
 * Returns: Error code for adding threads
 */
NewThreadStatus G8RTOS_AddThread(ThreadEntry_g entryFunc, uint8_t priority, const char name[MAX_TCB_NAME_LENGTH]) {
	static uint16_t idCounter = 0;
	
	G8RTOS_StartCriticalSection();

	if (numberThreads == MAX_THREADS) {
		G8RTOS_EndCriticalSection();
		return (NewThreadStatus) { .status = G8RTOS_TOO_MANY_THREADS, .id = NULL };
	}

	// Get first dead thread
	uint8_t newThreadIndex = 0;

	for (newThreadIndex = 0; newThreadIndex < MAX_THREADS; newThreadIndex++) {
		if (!threadControlBlocks[newThreadIndex].alive)
			break;
	}

	// Could not find an dead thread despite it not being full
	if (newThreadIndex == MAX_THREADS) {
		G8RTOS_EndCriticalSection();
		return (NewThreadStatus) { .status = G8RTOS_INCORRECTLY_ALIVE, .id = NULL };
	}

	TCB* newThread = &threadControlBlocks[newThreadIndex];
	ThreadID_t newThreadID = (idCounter++ << 16) | newThreadIndex;

	*newThread = (TCB) { 
		  .stackPtr   = &threadStacks[newThreadIndex][STACKSIZE - 16]
		, .previous   = (TCB*) NULL
		, .next       = (TCB*) NULL
		, .blockedSem = (Semaphore*) NULL
		, .sleepTime  = 0
		, .priority   = priority
		, .alive	  = TRUE
		, .id		  = newThreadID
		, .name  	  = "\0"
	};

	strcpy(newThread->name, name);
	
	// Build dummy stack frame
	newThread->stackPtr[15] = THUMB_BIT; // Ensure the xPSR field in the stack is set to thumb mode
	newThread->stackPtr[14] = (int32_t) entryFunc; // Set the to-return-to-PC to be the entry function
	newThread->stackPtr[13] = (int32_t) G8RTOS_KillSelf; // Set return value to be the KillSelf function
	newThread->stackPtr[11] = 0xDEADEAD;
	newThread->stackPtr[1] = 0xBEEFBEEF;
	newThread->stackPtr[0] = 0xFEABFEAB;

	// Insert the new thread into the running threads list.
	tcbll_Insert(&runningThreads, newThread);

	numberThreads++;

	G8RTOS_EndCriticalSection();

	return (NewThreadStatus) { .status = G8RTOS_SUCCESS, .id = newThreadID };
}

SchedulerStatus G8RTOS_AddPeriodicThread(ThreadEntry_g handler, uint_fast32_t period, uint_fast32_t firstStartTime) {
	G8RTOS_StartCriticalSection();

	if (numberPThreads == MAX_PTHREADS) {
		G8RTOS_EndCriticalSection();
		return G8RTOS_TOO_MANY_THREADS;
	}

	pthreads[numberPThreads] = (PTCB) {
		  .handler = handler
		, .period = period
		, .executionTime = firstStartTime
	};

	numberPThreads++;

	G8RTOS_EndCriticalSection();

	return G8RTOS_SUCCESS;
}

SchedulerStatus G8RTOS_AddAperiodicThread(ThreadEntry_g handler, uint8_t priority, int_fast32_t irqn) {
	G8RTOS_StartCriticalSection();
	
	if (irqn > (int_fast32_t) PORT6_IRQn || irqn < (int_fast32_t) PSS_IRQn) {
		G8RTOS_EndCriticalSection();
		return G8RTOS_INVALID_IRQN;
	}

	if (priority > 6) {
		G8RTOS_EndCriticalSection();
		return G8RTOS_INVALID_PRIORITY;
	}

	__NVIC_SetVector((IRQn_Type) irqn, (uint32_t) handler);
	NVIC_SetPriority((IRQn_Type) irqn, priority);
	NVIC_EnableIRQ((IRQn_Type) irqn);

	G8RTOS_EndCriticalSection();

	return G8RTOS_SUCCESS;
}

SchedulerStatus G8RTOS_KillThread(ThreadID_t id) {
	G8RTOS_StartCriticalSection();

	if (numberThreads == 1)
		return G8RTOS_CANNOT_KILL_LAST_THREAD;

	// Find thread with id
	uint16_t threadIndex = id & 0xFFFF; // Index is lower 2-bytes of the id
	
	TCB* toKill = &threadControlBlocks[threadIndex];

	// Make sure the thread exists, it is alive, and the ID matches
	if (threadIndex >= MAX_THREADS || toKill->id != id || !toKill->alive)
		return G8RTOS_THREAD_DOES_NOT_EXIST;

	if (IsThreadSleeping(toKill))
		tcbll_Remove(&sleepingThreads, toKill);
	else if (IsThreadBlocked(toKill))
		tcbll_Remove(&blockedThreads, toKill);
	else
		tcbll_Remove(&runningThreads, toKill);

	toKill->alive = FALSE; // Kill the thread
	numberThreads--;

	G8RTOS_EndCriticalSection();

	// Check if we are currently running thread. If we are, we need
	// to context switch
	if (toKill == currentThread) {
		TriggerContextSwitch();

		// Wait until the context switch is triggered
		// (sometimes does not happen immediately)
		while (TRUE) { };
		// Code flow will never reach this point.
	}

	return G8RTOS_SUCCESS;
}

SchedulerStatus G8RTOS_KillSelf() {
	return G8RTOS_KillThread(currentThread->id);
}

ThreadID_t G8RTOS_GetCurrentThreadID(void) {
	return currentThread->id;
}

void G8RTOS_SetThreadSwitchable(uint8_t canSwitch) {
	canContextSwitch = canSwitch;
}

bool_t G8RTOS_InPThread() {
	return inPThread;
}
