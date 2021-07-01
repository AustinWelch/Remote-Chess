
#include "uart.h"
#include <driverlib.h>


static const eUSCI_UART_Config Uart115200Config = {
    EUSCI_A_UART_CLOCKSOURCE_SMCLK
    , 6
    , 8
    , 0
    , EUSCI_A_UART_NO_PARITY
    , EUSCI_A_UART_LSB_FIRST
    , EUSCI_A_UART_ONE_STOP_BIT
    , EUSCI_A_UART_MODE
    , EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION
};

void uart_TransmitString(char* s) {
	while (*s) {
		MAP_UART_transmitData(EUSCI_A0_BASE, *s++);

	}
}

void uart_Init() {
	// Select GPIO functionality
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

	// Configure the digital oscillator
	CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);

	// Configure UART with 115200 baud
	MAP_UART_initModule(EUSCI_A0_BASE, &Uart115200Config);

	// Enable the UART
	MAP_UART_enableModule(EUSCI_A0_BASE);
}
