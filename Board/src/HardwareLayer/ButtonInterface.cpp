#include "ButtonInterface.h"
#include "msp.h"

using namespace RemoteChess;

// Up     - P8.3
// Down   - P9.1
// Left   - P9.3
// Right  - P6.3
// Center - P7.2


ButtonInterface::ButtonInterface() {
    // Init with pull-up resistors

    // Up
    P8->DIR &= ~BIT3; // Set to input
    P8->REN |= BIT3; // Enable resistor -> Bit = 1
    P8->OUT |= BIT3; // Pull-up -> Bit = 1

    // Down
    P9->DIR &= ~BIT1; // Set to input
    P9->REN |= BIT1; // Enable resistor -> Bit = 1
    P9->OUT |= BIT1; // Pull-up -> Bit = 1

    // Left
    P9->DIR &= ~BIT3; // Set to input
    P9->REN |= BIT3; // Enable resistor -> Bit = 1
    P9->OUT |= BIT3; // Pull-up -> Bit = 1

    // Right
    P6->DIR &= ~BIT3; // Set to input
    P6->REN |= BIT3; // Enable resistor -> Bit = 1
    P6->OUT |= BIT3; // Pull-up -> Bit = 1

    // Center
    P7->DIR &= ~BIT2; // Set to input
    P7->REN |= BIT2; // Enable resistor -> Bit = 1
    P7->OUT |= BIT2; // Pull-up -> Bit = 1
}

ButtonState ButtonInterface::GetCurrentButtonState() const {
    return {
          P8->IN & BIT3 // Up
        , P9->IN & BIT1 // Down
        , P9->IN & BIT3 // Left
        , P6->IN & BIT3 // Right
        , P7->IN & BIT2 // Center
    };
}