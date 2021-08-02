#pragma once

#include <cstdint>
#include <array>

namespace RemoteChess {
    class LCD_CharacterDisplay {
        private:
        static const uint8_t LCD_En = 0b00000100; // Enable bit
        static const uint8_t LCD_Rw = 0b00000010; // Read/~Write bit
        static const uint8_t LCD_Rs = 0b00000001; // Register select bit

        static const uint8_t LCD_Read = LCD_Rw;
        static const uint8_t LCD_Write = 0x00;

        // Function control
        static const uint8_t LCD_4BIT_MODE = 0x00;
        static const uint8_t LCD_2LINE = 0x08;
        static const uint8_t LCD_5x8DOTS = 0x00;

        // Display control
        static const uint8_t LCD_DISPLAY_ON = 0x04;
        static const uint8_t LCD_DISPLAY_OFF = 0x00;
        static const uint8_t LCD_CURSOR_ON = 0x02;
        static const uint8_t LCD_CURSOR_OFF = 0x00;
        static const uint8_t LCD_BLINK_ON = 0x01;
        static const uint8_t LCD_BLINK_OFF = 0x00;

        // Entry control
        static const uint8_t LCD_ENTRY_INC = 0x02;
        static const uint8_t LCD_ENTRY_SHIFT_DEC = 0x00;
        static const uint8_t LCD_ENTRY_SHIFT_INC = 0x01;

        // Expander control
        static const uint8_t BACKLIGHT_ON = 0x08;
        static const uint8_t BACKLIGHT_OFF = 0x00;



        enum class Command {
              CLEAR_DISPLAY   = 0x01
            , RETURN_HOME     = 0x02
            , ENTRY_MODE_SET  = 0x04
            , DISPLAY_CONTROL = 0x08
            , CURSOR_SHIFT    = 0x10
            , FUNCTION_SET    = 0x20
            , SET_CGRAM_ADDR  = 0x40
            , SET_DDRAM_ADDR  = 0x80
            , WRITE_DATA      = 0x00
        };

        uint8_t backlightVal = 0x08;

        public:
        LCD_CharacterDisplay();

        void ReturnHome() const;
        void Clear() const;
        void WriteMessage(std::array<const char*, 4> lines) const;
        void WriteMessageCenteredTitle(std::array<const char*, 4> lines) const;
        void WriteMessageWrapped(const char* msg) const;
        void WriteLine(const char* msg, uint8_t line) const;
        void WriteLineCentered(const char* msg, uint8_t line) const;
        void EnableBacklight();
        void DisableBacklight();

        private:
        void WriteToI2C(uint8_t byte) const; // Writes a single byte to I2C
        void ExpanderWrite(uint8_t data) const; // Writes to the I2C->8 bit expander with backlight value
        void Pulse(uint8_t data) const; // Holds the current command while toggling enable
        void WriteCommand(Command command, uint8_t data) const; // Command is sent via 4-bit protocol. Mode is Rw/Rs bits
        void WriteWithPulse(uint8_t data) const; // Write with the pulse wait
    };
}
