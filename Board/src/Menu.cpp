#include "Menu.h"
#include "msp.h"

using namespace RemoteChess;

Menu::Menu() {
    //TODO: Change Port Mapping
    P8->DIR &= ~(BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    P8->IFG &= ~(BIT3 | BIT4 | BIT5 | BIT6 | BIT7);
    P8->IES |= BIT3 | BIT4 | BIT5 | BIT6 | BIT7;
    P8->REN |= BIT3 | BIT4 | BIT5 | BIT6 | BIT7;
    P8->OUT |= BIT3 | BIT4 | BIT5 | BIT6 | BIT7;

    //Find IRQn for ports > 6
    __NVIC_SetPriority(PORT4_IRQn, 32);
    __NVIC_EnableIRQ(PORT4_IRQn);

}

uint8_t Menu::DisplayMenuLeft(LCD_CharacterDisplay &LCD, std::array<const char*, 4> selections, uint8_t startLine, uint8_t selectionSize) {
    for (int i = startLine; i < selectionSize + startLine; i++){
        char outChar[20];
        convertToChar(selections[i - startLine], outChar);
        LCD.WriteLineMenuLeft(outChar, i);
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
        char outChar[15];
        convertToChar(selections[2 * (i - startLine)], outChar);
        LCD.WriteLineMenuLeft(outChar, i);

        char outChar2[15];
        convertToChar(selections[2 * (i - startLine) + 1], outChar2);
        LCD.WriteLineMenuRight(outChar2, i);
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

        else if (input == ButtonDirection::DOWN) {
            if (currentPos / 2 != startLine + selectionSize - 1) {
                currentPos += 2;
                LCD.DrawCursor(currentPos, justification, prevPos, prevJustification);
            }
        }

        else 
            return currentPos;
    }
}

int8_t Menu::DisplayScrollingMenu(LCD_CharacterDisplay &LCD, flat_vector<std::string, 50> &scrollItems, uint8_t listSize, char* secondarySelection) {
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
            char name[20];
            Menu::convertToChar(scrollItems[(currentSelection / 4) * 4 + ((i - 1) * 2)], name);
            LCD.WriteLineMenuLeft(name, i);
            if ((listSize - (currentSelection / 4) * 4) != 3 || (listSize - (currentSelection / 4) * 4) != 1) {
                char name2[20];
                convertToChar(scrollItems[(currentSelection / 4) * 4 + (i * 2) - 1], name2);
                LCD.WriteLineMenuRight(name2, i);
            }
        }

        //TODO:erase prev lines

        input = AwaitInput();

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
                if (secondary)
                    currentPos = 6;
                else
                    currentPos = 7;
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

uint16_t Menu::getButtonInput() {
    return buttonInput;
}

uint16_t Menu::setButtonInput(uint16_t newVal) {
    buttonInput = newVal;
}

Menu::ButtonDirection Menu::AwaitInput() {
    //Could implement similar polling method except from multiple ports/pins
    P8->IFG = 0;
    P8->IE |= BIT3 | BIT4 | BIT5 | BIT6 | BIT7;

    while (!buttonInput);

    Menu::ButtonDirection retVal;

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

void Menu::convertToChar(std::string str, char* out) {
    for (int i = 0; i < str.size(); i++) {
        out[i] = str[i];
    }
}
