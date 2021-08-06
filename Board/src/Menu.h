#pragma once

#include "LCD_CharacterDisplay.h"
#include "flat_vector.h"
#include <string>

namespace RemoteChess {
    class Menu {
        enum class ButtonDirection {
              UP
            , RIGHT
            , DOWN 
            , LEFT 
            , CENTER
        };

        uint16_t buttonInput = 0;

        ButtonDirection AwaitInput();

        public:

        Menu();

        uint8_t DisplayMenuLeft(LCD_CharacterDisplay &LCD, std::array<const char*, 4> selections, uint8_t startLine, uint8_t selectionSize);
        uint8_t DisplayMenuLeftRight(LCD_CharacterDisplay &LCD, std::array<const char*, 8> selections, uint8_t startLine, uint8_t selectionSize);
        int8_t DisplayScrollingMenu(LCD_CharacterDisplay &LCD, flat_vector<std::string, 50> &scrollItems, uint8_t listSize, char* secondarySelection);

        uint16_t getButtonInput();
        uint16_t setButtonInput(uint16_t newVal);
        void convertToChar(std::string str, char* out);
    };
}
