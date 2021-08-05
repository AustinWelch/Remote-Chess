#include "Menu.h"

using namespace RemoteChess;

Menu::Menu() {
    P8->DIR &= ~(BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    P8->IFG &= ~(BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    P8->IES |= BIT3 | BIT4 | BIT5 | BIT6 | BIT7;
    P8->REN |= BIT3 | BIT4 | BIT5 | BIT6 | BIT7;
    P8->OUT |= BIT3 | BIT4 | BIT5 | BIT6 | BIT7;

    __NVIC_SetPriority(PORT8_IRQn, 32);
    __NVIC_EnableIRQ(PORT8_IRQn);
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

        input = AwaitInput;

        if (input == ButtonDirection::RIGHT || input == ButtonDirection::LEFT)
            continue;
        
        else if (input == ButtonDirection::UP){
            if (currentLine != startLine) {
                currentLine--;
                LCD.DrawCursor(currentLine, justification, prevLine, justification);
            }
        }

        else if (input == ButtonDirection::Down) {
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

        input = AwaitInput;

        if (input == ButtonDirection::RIGHT) {
            if (currentPos % 2 == 0) {
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
                LCD.DrawCursor(currentPos, justification, prevPos, prevJustification);
            }
        }

        else if (input == ButtonDirection::Down) {
            if (currentPos / 2 != startLine + selectionSize - 1) {
                currentPos += 2;
                LCD.DrawCursor(currentPos, justification, prevPos, prevJustification);
            }
        }

        else 
            return currentPos;
    }
}

int8_t Menu::DisplayScrollingMenu(LCD_CharacterDisplay &LCD, flat_vector<std::string, 50> scrollItems, uint8_t listSize, const char* secondarySelection) {
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
        prevPos = currentPos;
        prevJustification = justification;

        for (int i = 1; i < 3 || i < (listSize - (currentSelection / 4) * 4); i++) {
            LCD.WriteLineMenuLeft(scrollItems[(currentSelection / 4) * 4 + ((i - 1) * 2)], i);
            if ((listSize - (currentSelection / 4) * 4) != 3 || (listSize - (currentSelection / 4) * 4) != 1) 
                LCD.WriteLineMenuRight(scrollItems[(currentSelection / 4) * 4 + (i * 2) - 1], i);
        }

        //erase prev lines
        input = AwaitInput;

        if (input == ButtonDirection::RIGHT) {
            if (currentPos % 2 == 0) {
                if (currentPos != 6) {
                    currentPos++;
                    currentSelection++;
                    justification = LCD_CharacterDisplay::CursorJustify::RIGHT;
                    LCD.DrawCursor(currentPos / 2, justification, prevPos / 2, prevJustification);
                } else {
                    currentPos++;
                    justification = LCD_CharacterDisplay::CursorJustify::RIGHT;
                    LCD.DrawCursor(currentPos / 2, justification, prevPos / 2, prevJustification);
                }
            }
        }
    
        else if (input == ButtonDirection::LEFT) {
            if (currentPos % 2 == 1) {
                if (currentPos != 7) {
                    currentPos--;
                    currentSelection--;
                    justification = LCD_CharacterDisplay::CursorJustify::LEFT;
                    LCD.DrawCursor(currentPos / 2, justification, prevPos / 2, prevJustification);
                } else {
                    if (secondary) {
                        currentPos--;
                        justification = LCD_CharacterDisplay::CursorJustify::LEFT;
                        LCD.DrawCursor(currentPos / 2, justification, prevPos / 2, prevJustification);
                    }
                }
            } 
            else {
                justification = LCD_CharacterDisplay::CursorJustify::LEFT;
                secondary ? currentPos = 6; : currentPos = 7;
                LCD.DrawCursor(currentPos / 2, justification, prevPos / 2, prevJustification);
            }
        }
        
        else if (input == ButtonDirection::UP) {
            if (currentPos / 2 == startLine) {
                if (currentSelection >= 2)
                    currentSelection -= 2;  
            }
            else if (currentPos / 2 == 3) {
                currentSelection = (currentSelection / 4) * 4;
                currentPos = 2;
                LCD.DrawCursor(currentPos / 2, justification, prevPos / 2, prevJustification);
            }
            else {
                currentPos -= 2;
                LCD.DrawCursor(currentPos / 2, justification, prevPos / 2, prevJustification);
            }
        }

        else if (input == ButtonDirection::DOWN) {
            if (currentPos / 2 == 2) {
                if (currentSelection == listSize - 2) {
                    currentSelection += 1;
                } else {
                    currentSelection += 2;
                }
                currentPos -= 2;
            }
            else if (currentPos / 2 == 3) {
                continue;
            }
            else {
                currentPos += 2;
                LCD.DrawCursor(currentPos, justification, prevPos, prevJustification);
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

uint16_t getButtonInput() {
    return buttonInput;
}

ButtonDirection Menu::AwaitInput() {
    P8->IFG = 0;
    P8->IE |= BIT3 | BIT4 | BIT5 | BIT6 | BIT7;

    while (!buttonInput);

    ButtonDirection retVal;

    switch (buttonInput) {
        case BIT3:
            retVal = ButtonDirection::UP;
            break;
        case BIT4:
            retVal = ButtonDirection::RIGHT;
            break;
        case BIT5:
            retVal = ButtonDirection::DOWN;
            break;
        case BIT6:
            retVal = ButtonDirection::LEFT;
            break;
        case BIT7:
            retVal = ButtonDirection::CENTER;
            break;
        default:
            break;
    }

    buttonInput = 0;
    
    return retVal;
}

void PORT8_IRQHandler() {
    buttonInput = P8->IFG;

    P8->IFG &= ~(BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    P8->IE &= ~(BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
}