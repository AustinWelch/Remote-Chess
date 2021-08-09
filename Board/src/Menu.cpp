#include "Menu.h"
#include "msp.h"

#include <stdio.h>

using namespace RemoteChess;

Menu::Menu() {
    //TODO: Change Port Mapping
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

uint8_t Menu::DisplayMenuLeft(LCD_CharacterDisplay &LCD, std::array<const char*, 4> selections, uint8_t startLine, uint8_t selectionSize) {
    for (int i = startLine; i < selectionSize + startLine; i++){
        LCD.WriteLineMenuLeft(selections[i - startLine], i);
    }

    uint8_t currentLine = startLine;
    uint8_t prevLine = currentLine;
    auto justification = LCD_CharacterDisplay::CursorJustify::LEFT;

    ButtonDirection input;

    LCD.DrawCursor(startLine, justification, prevLine, justification);

    while (true) {
        prevLine = currentLine;

        input = AwaitInput();

        if (input == ButtonDirection::RIGHT || input == ButtonDirection::LEFT)
            continue;
        
        else if (input == ButtonDirection::UP){
            if (currentLine != startLine) {
                currentLine--;
                LCD.DrawCursor(currentLine, justification, prevLine, justification);
            }
        }

        else if (input == ButtonDirection::DOWN) {
            if (currentLine != startLine + selectionSize - 1) {
                currentLine++;
                LCD.DrawCursor(currentLine, justification, prevLine, justification);
            }
        }

        else 
            return currentLine;
    }
}

uint8_t Menu::DisplayMenuLeftRight(LCD_CharacterDisplay &LCD, std::array<const char*, 8> selections, uint8_t startLine, uint8_t selectionSize) {
    for (int i = startLine; i < (selectionSize + startLine * 2) / 2; i++){
        LCD.WriteLineMenuLeft(selections[2 * (i - startLine)], i);
        LCD.WriteLineMenuRight(selections[2 * (i - startLine) + 1], i);
    }

    if (selectionSize % 2 == 1){
        LCD.WriteLineMenuLeft(selections[selectionSize - 1], (selectionSize / 2) + startLine);
    }

    uint8_t currentPos = startLine * 2;
    uint8_t prevPos = currentPos;
    auto justification = LCD_CharacterDisplay::CursorJustify::LEFT;
    auto prevJustification = justification;

    ButtonDirection input;

    LCD.DrawCursor(startLine, justification, prevPos, justification);

    while (true) {
        prevPos = currentPos;
        prevJustification = justification;

        input = AwaitInput();

        if (input == ButtonDirection::RIGHT) {
            if (currentPos % 2 == 0 && currentPos - (startLine * 2) + 1 != selectionSize) {
                currentPos++;
                justification = LCD_CharacterDisplay::CursorJustify::RIGHT;
                LCD.DrawCursor(currentPos / 2, justification, prevPos / 2, prevJustification);
            }
        }
    
        else if (input == ButtonDirection::LEFT) {
            if (currentPos % 2 == 1) {
                currentPos--;
                justification = LCD_CharacterDisplay::CursorJustify::LEFT;
                LCD.DrawCursor(currentPos / 2, justification, prevPos / 2, prevJustification);
            }
        }
        
        else if (input == ButtonDirection::UP) {
            if (currentPos / 2 != startLine) {
                currentPos -= 2;
                LCD.DrawCursor(currentPos / 2, justification, prevPos / 2, prevJustification);
            }
        }

        else if (input == ButtonDirection::DOWN) {
            if (currentPos % 2 == 1 && selectionSize % 2 == 1 && currentPos - (startLine * 2) + 2 == selectionSize) {
                currentPos += 1;
                justification = LCD_CharacterDisplay::CursorJustify::LEFT;
                LCD.DrawCursor(currentPos / 2, justification, prevPos / 2, prevJustification);
            } else if (currentPos / 2 != startLine + (selectionSize / 2) - 1 + selectionSize % 2) {
                currentPos += 2;
                LCD.DrawCursor(currentPos / 2, justification, prevPos / 2, prevJustification);
            }
        }

        else 
            return currentPos;
    }
}

int8_t Menu::DisplayScrollingMenu(LCD_CharacterDisplay &LCD, flat_vector<char*, 10> &scrollItems, uint8_t listSize, char* secondarySelection) {
    LCD.WriteLineMenuRight("Back", 3);

    bool secondary = false;

    if(strlen(secondarySelection) > 1) {
        LCD.WriteLineMenuLeft(secondarySelection, 3);
        secondary = true;
    }

    uint8_t startLine = 1;

    uint8_t currentSelection = 0;
    uint8_t currentPos = 2;
    uint8_t prevPos = currentPos;
    auto justification = LCD_CharacterDisplay::CursorJustify::LEFT;
    auto prevJustification = justification;

    ButtonDirection input;

    LCD.DrawCursor(startLine, justification, prevPos, justification);

    while (true) {

        LCD.WriteLine("                    ", 1);
        LCD.WriteLine("                    ", 2);

        LCD.DrawCursor(currentPos / 2, justification, prevPos / 2, prevJustification);

        prevPos = currentPos;
        prevJustification = justification;

        for (int i = 1; i < 3; i++) {
            if (i >= (listSize - (currentSelection / 4) * 4) && (listSize != 1 || i > 1)) {
                break;
            }
            LCD.WriteLineMenuLeft(scrollItems[(currentSelection / 4) * 4 + ((i - 1) * 2)], i);
            if (((listSize - (currentSelection / 4) * 4) != 3 || (listSize - (currentSelection / 4) * 4) != 1) && listSize != 1) {
                LCD.WriteLineMenuRight(scrollItems[(currentSelection / 4) * 4 + (i * 2) - 1], i);
            }
        }

        input = AwaitInput();

        if (input == ButtonDirection::RIGHT) {
            if (currentPos % 2 == 0) {
                if (currentPos != 6) {
                    if (currentSelection + 1 != listSize) {
                        currentPos++;
                        currentSelection++;
                        justification = LCD_CharacterDisplay::CursorJustify::RIGHT;
                    }
                } else {
                    currentPos++;
                    justification = LCD_CharacterDisplay::CursorJustify::RIGHT;
                }
            }
        }
    
        else if (input == ButtonDirection::LEFT) {
            if (currentPos % 2 == 1) {
                if (currentPos != 7) {
                    currentPos--;
                    currentSelection--;
                    justification = LCD_CharacterDisplay::CursorJustify::LEFT;
                } else {
                    if (secondary) {
                        currentPos--;
                        justification = LCD_CharacterDisplay::CursorJustify::LEFT;
                    }
                }
            } 
            else {
                if (secondary) {
                    justification = LCD_CharacterDisplay::CursorJustify::LEFT;
                    currentPos = 6;
                } else {
                    justification = LCD_CharacterDisplay::CursorJustify::RIGHT;
                    currentPos = 7;
                }
            }
        }
        
        else if (input == ButtonDirection::UP) {
            if (currentPos / 2 == startLine) {
                if (currentSelection >= 2) {
                    currentPos += 2;
                    currentSelection -= 2;  
                }
            }
            else if (currentPos / 2 == 3) {
                currentSelection = (currentSelection / 4) * 4;
                justification = LCD_CharacterDisplay::CursorJustify::LEFT;
                currentPos = 2;
            }
            else {
                currentSelection -= 2;
                currentPos -= 2;
            }
        }

        else if (input == ButtonDirection::DOWN) {
            if (currentPos / 2 == 1) {
                if (currentPos % 2 == 0 && currentSelection + 2 <= listSize - 1) {
                    currentPos += 2;
                    currentSelection += 2;
                } else if (currentPos % 2 == 1) {
                    if (listSize - currentSelection - 1 >= 2) {
                        currentPos += 2;
                        currentSelection += 2;
                    } else if (listSize - currentSelection - 1 == 1) {
                        justification = LCD_CharacterDisplay::CursorJustify::LEFT;
                        currentPos++;
                        currentSelection++;
                    }
                }
            } else {
                if (currentPos % 2 == 0 && currentSelection + 2 <= listSize - 1) {
                    currentPos -= 2;
                    currentSelection += 2;
                } else if (currentPos % 2 == 1) {
                    if (listSize - currentSelection - 1 >= 2) {
                        currentPos -= 2;
                        currentSelection += 2;
                    } else if (listSize - currentSelection - 1 == 1) {
                        justification = LCD_CharacterDisplay::CursorJustify::LEFT;
                        currentPos -= 2;
                        currentSelection++;
                    }
                }
            }
        }

        else {
            if (currentPos == 7) 
                return -1;

            else if (currentPos == 6)
                return -2;

            return currentSelection;
        }
    }
}

uint16_t Menu::getButtonInput() {
    return buttonInput;
}

uint16_t Menu::setButtonInput(uint16_t newVal) {
    buttonInput = newVal;
}

Menu::ButtonDirection Menu::AwaitInput() {


    Menu::ButtonDirection retVal;

    char resp[10];


    while (true) {

//         if (!(P8->IN & BIT3)){ // Up
//             retVal = ButtonDirection::UP;
//             break;
//         }
//         if (!(P9->IN & BIT1)){ // Down
//             retVal = ButtonDirection::DOWN;
//            break;
//         }
//         if (!(P9->IN & BIT3)){// Left
//             retVal = ButtonDirection::LEFT;
//            break;
//         }
//         if (!(P6->IN & BIT3)){ // Right
//             retVal = ButtonDirection::RIGHT;
//             break;
//         }
//         if (!(P7->IN & BIT2)){ // Center
//             retVal = ButtonDirection::CENTER;
//             break;
//         }
//    }

        scanf("%s", resp);

        if(strstr(resp, "up")){
            retVal = ButtonDirection::UP;
            break;
        } else if(strstr(resp, "down")){
            retVal = ButtonDirection::DOWN;
            break;
        } else if(strstr(resp, "left")){
            retVal = ButtonDirection::LEFT;
            break;
        } else if(strstr(resp, "right")){
            retVal = ButtonDirection::RIGHT;
            break;
        } else if(strstr(resp, "center")){
            retVal = ButtonDirection::CENTER;
            break;
        }

//
//    buttonInput = 0;
//
    }
    return retVal;
}
