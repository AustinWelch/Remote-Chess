#include "LedMatrix.h"
#include "msp.h"

extern "C" {
    #include "G8RTOS_CriticalSection.h"
}

extern "C" void LED_Wait0High(void);
extern "C" void LED_Wait0Low(void);
extern "C" void LED_Wait1High(void);
extern "C" void LED_Wait1Low(void);

using namespace RemoteChess;

LedMatrix::LedMatrix() {
    P5->DIR |= BIT0;    
	P5->OUT |= BIT0; // Data line is defaulted to low (inverted in external circuit)

    DrawChecker();
}

void LedMatrix::SetCell(const Cell& cell, const RGB& color) {
    // Apply correction for direction switching every other row (due to LED data in looping around)
    if (cell.rank % 2 == 0) {
        grid[cell.rank][cell.file] = color;
    } else {
        grid[cell.rank][7 - cell.file] = color;
    }
}

void LedMatrix::SetCells(const RemoteChess::flat_vector<Cell, 32>& cells, const RGB& color) {
    for (const Cell& cell : cells) {
        SetCell(cell, color);
    }
}

void LedMatrix::SetAll(const RGB& color) {
    for (uint8_t i = 0; i < 8; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            grid[i][j] = color;
        }
    }
}

void LedMatrix::DrawChecker() {
    const RGB* checkerColor = &Colors::BLACK;

    for (uint8_t rank = 0; rank < 8; rank++) {
        for (uint8_t file = 0; file < 8; file++) {
            SetCell(Cell(file, rank), *checkerColor);

            if (checkerColor == &Colors::BLACK)
                checkerColor = &Colors::WHITE;
            else
                checkerColor = &Colors::BLACK;
        }

        if (checkerColor == &Colors::BLACK)
            checkerColor = &Colors::WHITE;
        else
            checkerColor = &Colors::BLACK;
    }
}

#define WS2812B_HIGH 0
#define WS2812B_LOW 1

void LedMatrix::Refresh() const {
    G8RTOS_StartCriticalSection();

    for (int rank = 0; rank < 8; rank++) {
        const auto& leds = grid[rank];

        for (int file = 0; file < 8; file++) {
            uint8_t outWord = leds[file].green;

            for (int i = 0 ; i < 8; i++) {
                if (outWord & 1) {
                    P5->OUT = WS2812B_HIGH;
                    LED_Wait0High();
                    P5->OUT = WS2812B_LOW;
                    LED_Wait0Low();
                } else {
                    P5->OUT = WS2812B_HIGH;
                    LED_Wait1High();
                    P5->OUT = WS2812B_LOW;
                    // LED_Wait1Low();
                }
                outWord >>= 1;
            }

            outWord = leds[file].red;

            for (int i = 0 ; i < 8; i++) {
                if (outWord & 1) {
                    P5->OUT = WS2812B_HIGH;
                    LED_Wait0High();
                    P5->OUT = WS2812B_LOW;
                    LED_Wait0Low();
                } else {
                    P5->OUT = WS2812B_HIGH;
                    LED_Wait1High();
                    P5->OUT = WS2812B_LOW;
                    // LED_Wait1Low();
                }
                outWord >>= 1;
            }

            outWord = leds[file].blue;

            for (int i = 0 ; i < 8; i++) {
                if (outWord & 1) {
                    P5->OUT = WS2812B_HIGH;
                    LED_Wait0High();
                    P5->OUT = WS2812B_LOW;
                    LED_Wait0Low();
                } else {
                    P5->OUT = WS2812B_HIGH;
                    LED_Wait1High();
                    P5->OUT = WS2812B_LOW;
                    // LED_Wait1Low();
                }
                outWord >>= 1;
            }
        }
    }

    P5->OUT = WS2812B_LOW;
    for (volatile int i = 0; i < 175; i++) { }

    G8RTOS_EndCriticalSection();
}

void LedMatrix::DrawRainbow() {
    SetCell(Cell(0, 0), Colors::RED);
    SetCell(Cell(1, 0), Colors::GREEN);
    SetCell(Cell(2, 0), Colors::BLUE);
    SetCell(Cell(3, 0), Colors::ORANGE);
    SetCell(Cell(4, 0), Colors::WHITE);
    SetCell(Cell(5, 0), Colors::MAGENTA);
    SetCell(Cell(6, 0), Colors::CYAN);
    SetCell(Cell(7, 0), Colors::YELLOW);

    SetCell(Cell(0, 1), Colors::RED);
    SetCell(Cell(1, 1), Colors::GREEN);
    SetCell(Cell(2, 1), Colors::BLUE);
    SetCell(Cell(3, 1), Colors::ORANGE);
    SetCell(Cell(4, 1), Colors::WHITE);
    SetCell(Cell(5, 1), Colors::MAGENTA);
    SetCell(Cell(6, 1), Colors::CYAN);
    SetCell(Cell(7, 1), Colors::YELLOW);

    SetCell(Cell(0, 2), Colors::RED);
    SetCell(Cell(1, 2), Colors::GREEN);
    SetCell(Cell(2, 2), Colors::BLUE);
    SetCell(Cell(3, 2), Colors::ORANGE);
    SetCell(Cell(4, 2), Colors::WHITE);
    SetCell(Cell(5, 2), Colors::MAGENTA);
    SetCell(Cell(6, 2), Colors::CYAN);
    SetCell(Cell(7, 2), Colors::YELLOW);

    SetCell(Cell(0, 3), Colors::RED);
    SetCell(Cell(1, 3), Colors::GREEN);
    SetCell(Cell(2, 3), Colors::BLUE);
    SetCell(Cell(3, 3), Colors::ORANGE);
    SetCell(Cell(4, 3), Colors::WHITE);
    SetCell(Cell(5, 3), Colors::MAGENTA);
    SetCell(Cell(6, 3), Colors::CYAN);
    SetCell(Cell(7, 3), Colors::YELLOW);

    SetCell(Cell(0, 4), Colors::RED);
    SetCell(Cell(1, 4), Colors::GREEN);
    SetCell(Cell(2, 4), Colors::BLUE);
    SetCell(Cell(3, 4), Colors::ORANGE);
    SetCell(Cell(4, 4), Colors::WHITE);
    SetCell(Cell(5, 4), Colors::MAGENTA);
    SetCell(Cell(6, 4), Colors::CYAN);
    SetCell(Cell(7, 4), Colors::YELLOW);

    SetCell(Cell(0, 5), Colors::RED);
    SetCell(Cell(1, 5), Colors::GREEN);
    SetCell(Cell(2, 5), Colors::BLUE);
    SetCell(Cell(3, 5), Colors::ORANGE);
    SetCell(Cell(4, 5), Colors::WHITE);
    SetCell(Cell(5, 5), Colors::MAGENTA);
    SetCell(Cell(6, 5), Colors::CYAN);
    SetCell(Cell(7, 5), Colors::YELLOW);

    SetCell(Cell(0, 6), Colors::RED);
    SetCell(Cell(1, 6), Colors::GREEN);
    SetCell(Cell(2, 6), Colors::BLUE);
    SetCell(Cell(3, 6), Colors::ORANGE);
    SetCell(Cell(4, 6), Colors::WHITE);
    SetCell(Cell(5, 6), Colors::MAGENTA);
    SetCell(Cell(6, 6), Colors::CYAN);
    SetCell(Cell(7, 6), Colors::YELLOW);

    SetCell(Cell(0, 7), Colors::RED);
    SetCell(Cell(1, 7), Colors::GREEN);
    SetCell(Cell(2, 7), Colors::BLUE);
    SetCell(Cell(3, 7), Colors::ORANGE);
    SetCell(Cell(4, 7), Colors::WHITE);
    SetCell(Cell(5, 7), Colors::MAGENTA);
    SetCell(Cell(6, 7), Colors::CYAN);
    SetCell(Cell(7, 7), Colors::YELLOW);
}

