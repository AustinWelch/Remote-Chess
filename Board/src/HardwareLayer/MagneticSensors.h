#pragma once

#include <array>
#include "Cell.h"
#include "ChessBoard.h"

extern "C" {
    #include "G8RTOS_Semaphores.h"
}

namespace RemoteChess {

    struct MagneticSensors {
        using CellValue_g = void(const Cell&, bool);

        private:
        std::array<std::array<bool, 8>, 8> currentMagnetValues; // [file][rank]
        mutable Semaphore sensorSem = { 1 };

        public:
        MagneticSensors();

        void UpdateMagnetValuesAndPropagate(ChessBoard& board);

        decltype(currentMagnetValues) GetCurrentMagnetValues() const;

        private:
        static uint8_t ConvertFileToMuxSel(uint8_t file);

        void MeasureInitialMagnetValues();
        std::array<bool, 8> SenseFile(uint8_t muxSel) const;
    };
}
