/*
 * ClockSys.c
 *
 * Holds all functionality for modifying Clock System
 *
 *  Created on: Dec 30, 2016
 *      Author: Raz Aloni
 */

#include <stdint.h>
#include <driverlib.h>

#include "ClockSys.h"

/********************************** Public Functions **************************************/

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
void ClockSys_SetMaxFreq()
{
	/* Set GPIO to be Crystal In/Out for HFXT */
	MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_PJ, GPIO_PIN3 | GPIO_PIN2, GPIO_PRIMARY_MODULE_FUNCTION);

	/* Set Core Voltage Level to VCORE1 to handle 48 MHz Speed */
	while(!PCM_setCoreVoltageLevel(PCM_VCORE1));

	/* Set frequency of HFXT and LFXT */
	MAP_CS_setExternalClockSourceFrequency(32000, 48000000);

	/* Set 2 Flash Wait States */
	MAP_FlashCtl_setWaitState(FLASH_BANK0, 2);
	MAP_FlashCtl_setWaitState(FLASH_BANK1, 2);

	/* Danny added this for Wi-Fi */
	FLCTL->BANK0_RDCTL |= (FLCTL_BANK0_RDCTL_BUFI | FLCTL_BANK0_RDCTL_BUFD );
    FLCTL->BANK1_RDCTL |= (FLCTL_BANK1_RDCTL_BUFI | FLCTL_BANK1_RDCTL_BUFD );

	/* Start HFXT */
	MAP_CS_startHFXT(0);

	/* Initialize MCLK to HFXT */
	MAP_CS_initClockSignal(CS_MCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_1);

	/* Initialize HSMCLK to HFXT/2 */
	MAP_CS_initClockSignal(CS_HSMCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_2);

	/* Initialize SMCLK to HFXT/4 */
	MAP_CS_initClockSignal(CS_SMCLK, CS_HFXTCLK_SELECT, CS_CLOCK_DIVIDER_4);
}

/* Gets the Main Clock System Frequency */
uint32_t ClockSys_GetSysFreq()
{
	return MAP_CS_getMCLK();
}

/********************************** Public Functions **************************************/
