/*
 * BSP.c
 *
 *  Created on: Dec 30, 2016
 *      Author: Raz Aloni
 */

#include <driverlib.h>
#include "BSP.h"
#include "i2c_driver.h"
#include "uart.h"


/* Initializes the entire board */
void BSP_InitBoard()
{
	/* Disable Watchdog */
	WDT_A_clearTimer();
	WDT_A_holdTimer();

	/* Initialize Clock */
	ClockSys_SetMaxFreq();

	/* Init i2c */
	//initI2C();

	uart_Init();

}


