/*
 * BackChannelUart.c
 * Holds all logic and functionality for communicating via the Back Channel UART
 * The Back Channel UART is configured to run at 115200 Baud with 1 start bit, 8 data bits, 1 stop bit, and no parity bit
 * The Back Channel UART will transmit board related data in JSON to be read by the COM port receiver (Host PC)
 *  Created on: Jan 4, 2017
 *      Author: Raz Aloni
 */

/******************************************* Includes ************************************/

#include <stdint.h>
#include <driverlib.h>
#include <stdio.h>
#include "BackChannelUart.h"


/******************************************* Includes ************************************/


/******************************************* Defines *************************************/

#define SBUFF_SIZE 255

/******************************************* Defines *************************************/


/******************************************* Private Variables ***************************/

/* Configuratin for Backchannel UART */
static const eUSCI_UART_Config backChannelUart115200Config =
{
		EUSCI_A_UART_CLOCKSOURCE_SMCLK, 				// SMCLK Clock Source
		6, 												// BRDIV
		8, 												// UCxBRF
		0, 												// UCxBRS
		EUSCI_A_UART_NO_PARITY, 						// No Parity
		EUSCI_A_UART_LSB_FIRST, 						// LSB First
		EUSCI_A_UART_ONE_STOP_BIT, 						// One stop bit
		EUSCI_A_UART_MODE, 								// UART mode
		EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION 	// Oversampling
};

/* Buffer for holding created strings */
static char backChannelStringBuff[SBUFF_SIZE];

/******************************************* Private Variables ***************************/


/******************************************* Private Functions ***************************/

/*
 * Transmits a string over UART
 * Param 's': A null-terminated C-string
 */
static inline void BackChannelTransmitString(char * s)
{
	/* Loop while not null */
	while(*s)
	{
		MAP_UART_transmitData(EUSCI_A0_BASE, *s++);
	}
}

/******************************************* Private Functions ***************************/


/******************************************* Public Functions ****************************/

/* Initializes back channel UART */
void BackChannelInit()
{
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
	MAP_UART_initModule(EUSCI_A0_BASE, &backChannelUart115200Config);
	MAP_UART_enableModule(EUSCI_A0_BASE);
}

/*
 * Prints string to the back channel UART
 * Param 'string': String to be displayed
 * Param 'textStyle': Style of the the text to be written
 */
void BackChannelPrint(const char * string, BackChannelTextStyle_t textStyle)
{
	char * topic;

	/* Set topic of JSON */
	switch(textStyle)
	{
		case BackChannel_Warning:
		{
			topic = "warning";
			break;
		}
		case BackChannel_Error:
		{
			topic = "error";
			break;
		}
		case BackChannel_Info:
		default:
		{
			topic = "info";
		}
	}

	snprintf(backChannelStringBuff, SBUFF_SIZE, "{ \"%s\" : \"%s\" }\r\n", topic, string);

	BackChannelTransmitString(backChannelStringBuff);
}

/*
 * Prints the value of an integer to the back channel UART
 * Param 'name': Name of the integer variable
 * Param 'value': Value of integer variable
 */
void BackChannelPrintIntVariable(const char * name, int32_t value)
{
	snprintf(backChannelStringBuff, SBUFF_SIZE, "{ \"variable\" : { \"name\" : \"%s\", \"value\" : %d } }\r\n", name, value);
	BackChannelTransmitString(backChannelStringBuff);
}

/*
 * Sends an event trigger to the back channel UART
 * Param 'eventNumber': Event number indicator
 */
void BackChannelEventTrigger(uint_fast8_t eventNumber)
{
	snprintf(backChannelStringBuff, SBUFF_SIZE, "{ \"event\" : %d }\r\n", eventNumber);
	BackChannelTransmitString(backChannelStringBuff);
}

/******************************************* Public Functions ****************************/


