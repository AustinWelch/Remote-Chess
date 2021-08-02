/*
 * G8RTOS_CriticalSection.h
 *
 *  Created on: Dec 11, 2016
 *      Author: Raz Aloni
 */

#ifndef G8RTOS_CRITICALSECTION_H_
#define G8RTOS_CRITICALSECTION_H_

/*
 * Starts a critical section
 * 	- Saves the state of the current PRIMASK (I-bit)
 * 	- Disables interrupts
 * Returns: The current PRIMASK State
 */
extern void G8RTOS_StartCriticalSection(void);

/*
 * Ends a critical Section
 * 	- Restores the state of the PRIMASK given an input
 * Param "IBit_State": PRIMASK State to update
 */
extern void G8RTOS_EndCriticalSection(void);


#endif /* G8RTOS_CRITICALSECTION_H_ */
