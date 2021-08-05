#include "LCD_CharacterDisplay.h"

extern "C" {
    #include "G8RTOS_Scheduler.h"
    #include "G8RTOS_CriticalSection.h"
    #include "msp.h"
    #include "i2c.h"
}

using namespace RemoteChess;


LCD_CharacterDisplay::LCD_CharacterDisplay() {
    // 6.4 is SDA
    // 6.5 is SCL

    P6->SEL0 |= BIT4 | BIT5;
    P6->SEL1 &= ~(BIT4 | BIT5);

    eUSCI_I2C_MasterConfig config {
          EUSCI_B_I2C_CLOCKSOURCE_SMCLK
        , 12000000
        , EUSCI_B_I2C_SET_DATA_RATE_100KBPS
        , 0
        , EUSCI_B_I2C_NO_AUTO_STOP
    };

    I2C_initMaster(EUSCI_B1_BASE, &config);
    I2C_setSlaveAddress(EUSCI_B1_BASE, 0x27);
    I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
    I2C_enableModule(EUSCI_B1_BASE);


    ExpanderWrite(0); // Write backlight

    // Initializing by Instruction, three tries
    WriteWithPulse(0x03 << 4);
    G8RTOS_SleepThread(5); // Wait at least 4.5ms
    WriteWithPulse(0x03 << 4);
    G8RTOS_SleepThread(5); // Wait at least 4.5ms
    WriteWithPulse(0x03 << 4);
    G8RTOS_SleepThread(1); // Wait at least 150us

    G8RTOS_SleepThread(50);

    G8RTOS_StartCriticalSection();

    WriteWithPulse(((uint8_t) Command::FUNCTION_SET) | LCD_4BIT_MODE);

    G8RTOS_EndCriticalSection();

    uint8_t functionControl = LCD_4BIT_MODE | LCD_2LINE | LCD_5x8DOTS;
    WriteCommand(Command::FUNCTION_SET, functionControl);

    //WriteCommand(Command::DISPLAY_CONTROL, LCD_DISPLAY_OFF);

    //Clear();

    uint8_t displayControl = LCD_DISPLAY_ON | LCD_CURSOR_ON;
    WriteCommand(Command::DISPLAY_CONTROL, displayControl);

    uint8_t entryControl = LCD_ENTRY_INC;
    WriteCommand(Command::ENTRY_MODE_SET, entryControl);

    // Initialization complete

    Clear();
    ReturnHome();
}

void LCD_CharacterDisplay::ReturnHome() const {
    WriteCommand(Command::RETURN_HOME, 0);
    G8RTOS_SleepThread(2);
}

void LCD_CharacterDisplay::Clear() const {
    WriteCommand(Command::CLEAR_DISPLAY, 0);
    G8RTOS_SleepThread(2);
}

void LCD_CharacterDisplay::WriteLine(const char* msg, uint8_t line) const {
    static uint8_t lineAddresses[4] = { 0x00, 0x40, 0x14, 0x54 };

    WriteCommand(Command::SET_DDRAM_ADDR, lineAddresses[line]);

    for (uint8_t i = 0; i < 20; i++) {
        if (msg[i] == '\0') break;

        WriteCommand(Command::WRITE_DATA, msg[i]); 
    }
}

void LCD_CharacterDisplay::WriteLineMenuLeft(const char* msg, uint8_t line) const {
    static uint8_t lineAddresses[4] = { 0x01, 0x41, 0x15, 0x55 };

    WriteCommand(Command::SET_DDRAM_ADDR, lineAddresses[line]);

    for (uint8_t i = 0; i < 20; i++) {
        if (msg[i] == '\0') break;

        WriteCommand(Command::WRITE_DATA, msg[i]); 
    }
}

void LCD_CharacterDisplay::WriteLineMenuRight(const char* msg, uint8_t line) const {
    static uint8_t lineAddresses[4] = { 0x0C, 0x4C, 0x20, 0x60 };

    WriteCommand(Command::SET_DDRAM_ADDR, lineAddresses[line]);

    for (uint8_t i = 0; i < 20; i++) {
        if (msg[i] == '\0') break;

        WriteCommand(Command::WRITE_DATA, msg[i]); 
    }
}

void LCD_CharacterDisplay::DrawCursor(uint8_t line, CursorJustify justification, uint8_t prevLine, CursorJustify prevJustification) {
    static uint8_t cursorAddresses[8] = { 0x00, 0x40, 0x14, 0x54, 0x0B, 0x4B, 0x1F, 0x5F};

    if (prevJustification == CursorJustify::LEFT)
        WriteChar(' ', cursorAddresses[prevLine]);
    else 
        WriteChar(' ', cursorAddresses[prevLine + 4]);

    if (justification == CursorJustify::LEFT)
        WriteChar('>', cursorAddresses[line]);
    else 
        WriteChar('>', cursorAddresses[line + 4]);
}

void LCD_CharacterDisplay::WriteChar(const char ch, uint8_t pos) const {
    WriteCommand(Command::SET_DDRAM_ADDR, pos);
    WriteCommand(Command::WRITE_DATA, ch); 
}

void LCD_CharacterDisplay::WriteLineCentered(const char* msg, uint8_t line) const {
    static uint8_t lineCentersAddresses[4] = { 0x0A, 0x4A, 0x1E, 0x5E };
    uint8_t offset = strlen(msg) / 2;
    
    WriteCommand(Command::SET_DDRAM_ADDR, lineCentersAddresses[line] - offset);

    for (uint8_t i = 0; i < 20; i++) {
        if (msg[i] == '\0') break;

        WriteCommand(Command::WRITE_DATA, msg[i]); 
    }
}

void LCD_CharacterDisplay::WriteMessage(std::array<const char*, 4> lines) const {
    for (uint8_t i = 0; i < 4; i++) {
        WriteLine(lines[i], i);
    }
}

void LCD_CharacterDisplay::WriteMessageCenteredTitle(std::array<const char*, 4> lines) const {
    WriteLineCentered(lines[0], 0);
    for (uint8_t i = 1; i < 4; i++) {
        WriteLine(lines[i], i);
    }
}

void LCD_CharacterDisplay::WriteMessageWrapped(const char* msg) const {
    static uint8_t lineAddresses[4] = { 0x00, 0x40, 0x14, 0x54 };
    //char* word[20]; add word wrapping
    char lineText[20];

    uint16_t msgLength = strlen(msg);
    uint8_t line = 0;
    uint16_t j;
    for (uint16_t i = 0; i < msgLength; i++) {
        j = i - (line * 20);
        lineText[j] = msg[i];

        if (j % 20 == 19) {
            WriteLine(lineText, line);
            line++;
        } 
    }
}

void LCD_CharacterDisplay::WriteToI2C(uint8_t byte) const {
    I2C_masterSendSingleByte(EUSCI_B1_BASE, byte);
}

void LCD_CharacterDisplay::ExpanderWrite(uint8_t data) const {
    WriteToI2C(data | backlightVal);
}

extern "C" void LED_Wait1High(void); // Steal a wait for 900ns function

void LCD_CharacterDisplay::Pulse(uint8_t data) const {
    ExpanderWrite(data | LCD_En); // Enable
    LED_Wait1High(); // Hold command for at least 450ns

    ExpanderWrite(data & ~LCD_En); // Turn off enable
    for (volatile int i = 0; i < 175; i++) { } // Wait > 37us in between commands
}

void LCD_CharacterDisplay::WriteWithPulse(uint8_t data) const {
    ExpanderWrite(data);
    Pulse(data);
}

void LCD_CharacterDisplay::WriteCommand(Command command, uint8_t data) const {

    uint8_t packet = ((uint8_t) command) | data;
    uint8_t upperPktNib = packet & 0xF0;
    uint8_t lowerPktNib = (packet << 4) & 0xF0;
    
    uint8_t mode = command == Command::WRITE_DATA ? LCD_Rs : 0;

    G8RTOS_StartCriticalSection();

    WriteWithPulse(upperPktNib | mode);
    WriteWithPulse(lowerPktNib | mode);

    G8RTOS_EndCriticalSection();
}

void LCD_CharacterDisplay::EnableBacklight() {
    backlightVal = BACKLIGHT_ON;
    ExpanderWrite(0);
}

void LCD_CharacterDisplay::DisableBacklight() {
    backlightVal = BACKLIGHT_OFF;
    ExpanderWrite(0);
}

