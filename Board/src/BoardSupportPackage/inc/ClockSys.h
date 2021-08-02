/*
 * ClockSys.h
 * Holds all functionality for modifying Clock System
 *
 *  Created on: Dec 30, 2016
 *      Author: Raz Aloni
 */

#ifndef CLOCKSYS_H_
#define CLOCKSYS_H_

/*
 * Initializes Core Clock to Maximum Frequency with highest accuracy
 * 	Initializes GPIO for HFXT in and out
 * 	Enables HFXT
 * 	Sets MSP432 to Power Active Mode to handle 48 MHz
 *  Sets Flash Wait states for 48 MHz
 *  Sets MCLK to 48 MHz
 *  Sets HSMCLK to 24 MHz
 *  Sets SMCLK to 12 MHz
 */
extern void ClockSys_SetMaxFreq();

/* Gets the Main Clock System Frequency */
extern uint32_t ClockSys_GetSysFreq();

#endif /* CLOCKSYS_H_ */
