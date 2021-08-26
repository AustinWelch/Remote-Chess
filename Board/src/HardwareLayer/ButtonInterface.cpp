#include "ButtonInterface.h"
#include "msp.h"

extern "C" {
    #include "G8RTOS_Scheduler.h"
}

using namespace RemoteChess;

// Interrupt for each button
// On button down, it marks a flag in ButtonInterface -> Detected falling edge
// In GetButtonState, check detected falling edge AND then wait a ms
// Return falling edge && debounce check
// Clear falling edge

// Up     - P8.3
// Down   - P6.3
// Left   - P9.1
// Right  - P9.3
// Center - P7.2


ButtonInterface::ButtonInterface() {
    // Init with pull-up resistors
    
    // Up
    P8->DIR &= ~BIT3; // Set to input
    P8->IFG &= ~BIT3; //Clear flag
    P8->IES |= BIT3; // High-to-Low transition
    P8->REN |= BIT3; // Enable resistor -> Bit = 1
    P8->OUT |= BIT3; // Pull-up -> Bit = 1
    P8->IE |= BIT3; // Enable interrupt

    // Left
    P9->DIR &= ~BIT1; // Set to input
    P9->IFG &= ~BIT1; //Clear flag
    P9->IES |= BIT1; // High-to-Low transition
    P9->REN |= BIT1; // Enable resistor -> Bit = 1
    P9->OUT |= BIT1; // Pull-up -> Bit = 1
    P9->IE |= BIT1; // Enable interrupt

    // Right
    P9->DIR &= ~BIT3; // Set to input
    P9->IFG &= ~BIT3; //Clear flag
    P9->IES |= BIT3; // High-to-Low transition
    P9->REN |= BIT3; // Enable resistor -> Bit = 1
    P9->OUT |= BIT3; // Pull-up -> Bit = 1
    P9->IE |= BIT3; // Enable interrupt

    // Down
    P6->DIR &= ~BIT3; // Set to input
    P6->IFG &= ~BIT3; //Clear flag
    P6->IES |= BIT3; // High-to-Low transition
    P6->REN |= BIT3; // Enable resistor -> Bit = 1
    P6->OUT |= BIT3; // Pull-up -> Bit = 1
    P6->IE |= BIT3; // Enable interrupt

    // Center
    P7->DIR &= ~BIT2; // Set to input
    P7->IFG &= ~BIT2; //Clear flag
    P7->IES |= BIT2; // High-to-Low transition
    P7->REN |= BIT2; // Enable resistor -> Bit = 1
    P7->OUT |= BIT2; // Pull-up -> Bit = 1
    P7->IE |= BIT2; // Enable interrupt
}

// Delays for 1ms for debouncing
ButtonState ButtonInterface::GetCurrentButtonState() {
    ButtonState before = {
          !(P8->IN & BIT3) // Up
        , !(P6->IN & BIT3) // Down
        , !(P9->IN & BIT1) // Left
        , !(P9->IN & BIT3) // Right
        , !(P7->IN & BIT2) // Center
    };

    G8RTOS_SleepThread(100);

    ButtonState after = {
          !(P8->IN & BIT3) // Up
        , !(P6->IN & BIT3) // Down
        , !(P9->IN & BIT1) // Left
        , !(P9->IN & BIT3) // Right
        , !(P7->IN & BIT2) // Center
    };

    return {
          !(before.up)     && after.up     // Up
        , !(before.down)   && after.down   // Down
        , !(before.left)   && after.left   // Left
        , !(before.right)  && after.right  // Right
        , !(before.center) && after.center // Center
    };
}

ButtonState ButtonInterface::GetCurrentButtonStatePoll() const {
   ButtonState before = {
          !(P8->IN & BIT3) // Up
        , !(P6->IN & BIT3) // Down
        , !(P9->IN & BIT1) // Left
        , !(P9->IN & BIT3) // Right
        , !(P7->IN & BIT2) // Center
    };

    G8RTOS_SleepThread(1);

    ButtonState after = {
          !(P8->IN & BIT3) // Up
        , !(P6->IN & BIT3) // Down
        , !(P9->IN & BIT1) // Left
        , !(P9->IN & BIT3) // Right
        , !(P7->IN & BIT2) // Center
    };

    return {
          before.up     && after.up     // Up
        , before.down   && after.down   // Down
        , before.left   && after.left   // Left
        , before.right  && after.right  // Right
        , before.center && after.center // Center
    };
}


void ButtonInterface::resetButtons() {
    buttons.up = false;
    buttons.down = false;
    buttons.left = false;
    buttons.right = false;
    buttons.center = false;

    P6->IFG &= ~BIT3;
    P7->IFG &= ~BIT2;
    P8->IFG &= ~BIT3;
    P9->IFG &= ~BIT1;
    P9->IFG &= ~BIT3;
}

bool ButtonInterface::isButtonPressed() {
    return !(buttons.up) // Up
        || !(buttons.down) // Down
        || !(buttons.left) // Left
        || !(buttons.right) // Right
        || !(buttons.center); // Center
}
