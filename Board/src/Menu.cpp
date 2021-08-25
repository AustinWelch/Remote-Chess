#include "Menu.h"
#include "msp.h"

#include <stdio.h>

extern "C" {
    #include "G8RTOS_Scheduler.h"
}

using namespace RemoteChess;

Menu::Menu() { }

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
            return currentLine - startLine;
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
            return currentPos - startLine * 2;
    }
}

int8_t Menu::DisplayScrollingMenu(LCD_CharacterDisplay &LCD, const flat_vector<const char*, 10>& scrollItems, uint8_t listSize, const char* secondarySelection) {
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

    while (true) {

        LCD.ClearLine(1);
        LCD.ClearLine(2);

        LCD.DrawCursor(currentPos / 2, justification, prevPos / 2, prevJustification);

        prevPos = currentPos;
        prevJustification = justification;

        for (int i = 1; i < 3; i++) {
            if (i >= (listSize - (currentSelection / 4) * 4) && (i > 1 || (listSize - (currentSelection / 4) * 4) != 1)) {
                break;
            }
            LCD.WriteLineMenuLeft(scrollItems[(currentSelection / 4) * 4 + ((i - 1) * 2)], i);
            if (((listSize - (currentSelection / 4) * 4) != 3 && (listSize - (currentSelection / 4) * 4) != 1)) {
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

int8_t Menu::KeyboardInputNumber(LCD_CharacterDisplay &LCD, const char* firstString, char* returnString) {
    static uint8_t lineAddresses[4] = { 0x00, 0x40, 0x14, 0x54 };
    LCD.Clear();
    LCD.WriteLine("0123456789", 1);
    LCD.WriteCharAtCurPos(0x7F);

    LCD.EnableLCDCursor();
    LCD.WriteLineMenuLeft("Enter", 3);
    LCD.WriteLineMenuRight("Back", 3);

    uint8_t stringPos = 0;
    int16_t currentSelection = 0;
    int16_t prevSelection = 0;
    uint16_t currentLine = 1;
    auto justification = LCD_CharacterDisplay::CursorJustify::LEFT;
    LCD_CharacterDisplay::CursorJustify prevJustification;

    ButtonDirection input;
    
    char previewOutput[20];

    while (true) {
        LCD.WriteLine("                   ", 0); //Clear line
        sprintf(previewOutput, "%s%s", firstString, returnString);
        LCD.WriteLine(previewOutput, 0);

        LCD.SetLCDAddr(lineAddresses[1] + currentSelection);

        input = AwaitInput();

        if (input == ButtonDirection::UP) {
            if (currentLine == 3) {
                LCD.EnableLCDCursor();
                currentSelection = prevSelection;
                currentLine = 1;

                uint8_t erasePos = lineAddresses[3];
                if (prevJustification == LCD_CharacterDisplay::CursorJustify::RIGHT) 
                    erasePos + 11;

                LCD.WriteChar(' ', erasePos);
            }
        } 
        
        else if (input == ButtonDirection::DOWN) {
            if (currentLine == 1) {
                prevSelection = currentSelection;
                LCD.DisableLCDCursor();
                currentSelection = -1;
                currentLine = 3;
                justification = LCD_CharacterDisplay::CursorJustify::LEFT;
                LCD.DrawCursor(currentLine, justification, currentLine, prevJustification);
            }
        }

        else if (input == ButtonDirection::LEFT) {
            if (currentLine == 1 && currentSelection != 0) {
                currentSelection--;
            } else if (currentLine == 3 && currentSelection == -2) {
                currentSelection = -1;
                justification = LCD_CharacterDisplay::CursorJustify::LEFT;
                LCD.DrawCursor(currentLine, justification, currentLine, prevJustification);
            }
        }

        else if (input == ButtonDirection::RIGHT) {
            if (currentLine == 1 && currentSelection != 10) {
                currentSelection++;
            } else if (currentLine == 3 && currentSelection == -1) {
                currentSelection = -2;
                justification = LCD_CharacterDisplay::CursorJustify::RIGHT;
                LCD.DrawCursor(currentLine, justification, currentLine, prevJustification);
            }
        }

        else { //Center
            if (currentSelection == -2) {
                return 0; //Back
            } else if (currentSelection == -1) {
                returnString[stringPos] = '\0';
                return 1; //Submit
            } 

            if (currentSelection == 10 && stringPos != 0) {
                returnString[--stringPos] = '\0';
            } else if (stringPos < sizeof(returnString)/8 - 1) {
                returnString[stringPos++] = currentSelection + 48; 
            }
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
    extern ButtonInterface g_buttons;
    Menu::ButtonDirection retVal;

    while (true) {
        ButtonState btnState = g_buttons.GetCurrentButtonState();

        if(btnState.up) {
            retVal = ButtonDirection::UP;
            break;
        } else if (btnState.down) {
            retVal = ButtonDirection::DOWN;
            break;
        } else if(btnState.left) {
            retVal = ButtonDirection::LEFT;
            break;
        } else if(btnState.right){
            retVal = ButtonDirection::RIGHT;
            break;
        } else if(btnState.center){
            retVal = ButtonDirection::CENTER;
            break;
        }
    }
    return retVal;
}
