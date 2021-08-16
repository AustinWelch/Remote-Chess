#pragma once

#include <array>
#include "Cell.h"

extern "C" {
    #include "G8RTOS_Semaphores.h"
}

namespace RemoteChess {
    class Board;

    struct MagneticSensors {
        using CellValue_g = void(const Cell&, bool);

        private:
        std::array<std::array<bool, 8>, 8> currentMagnetValues; // [file][rank]
        mutable Semaphore sensorSem = { 1 };

        public:
        MagneticSensors();

        void MeasureInitialMagnetValues();
        void UpdateMagnetValuesAndPropagate(Board& board);

        decltype(currentMagnetValues) GetCurrentMagnetValues() const; // [file][rank]

        private:
        static uint8_t ConvertFileToMuxSel(uint8_t file);
        static void MagneticSensors::SetMuxSel(uint8_t file);

        std::array<bool, 8> SenseFile(uint8_t muxSel) const;
    };
}