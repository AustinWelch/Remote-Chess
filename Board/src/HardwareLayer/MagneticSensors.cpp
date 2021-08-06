#include "MagneticSensors.h"
#include "../ChessBoard.h"

extern "C" {
    #include "msp.h"
    #include "G8RTOS_Scheduler.h"
}

using namespace RemoteChess;

MagneticSensors::MagneticSensors() {
	// Port 5, 0-2 select lines
	P5->DIR = BIT0 | BIT1 | BIT2;

    MeasureInitialMagnetValues();
}

uint8_t MagneticSensors::ConvertFileToMuxSel(uint8_t file) {
	switch (file) {
		case 0:
			return 7;
		case 1:
			return 4;
		case 2:
			return 5;
		case 3:
			return 6;
		case 4:
			return 0;
		case 5:
			return 1;
		case 6:
			return 2;
		case 7:
			return 3;
	}
}

std::array<bool, 8> MagneticSensors::SenseFile(uint8_t file) const {
    uint8_t muxSel = ConvertFileToMuxSel(file);

    P5->OUT = muxSel;

    return {
          !(P2->IN & BIT7) // Rank 0
        , !(P2->IN & BIT6)
        , !(P2->IN & BIT5)
        , !(P2->IN & BIT4)
        , !(P2->IN & BIT3)
        , !(P4->IN & BIT7)
        , !(P4->IN & BIT6)
        , !(P4->IN & BIT5) // Rank 7
    };
}

void MagneticSensors::MeasureInitialMagnetValues() {
    for (uint8_t file = 0; file < 8; file++) {
        auto fileValues = SenseFile(file); 

        for (volatile int i = 0; i < 30000; i++) {

        }

        currentMagnetValues[file] = fileValues;
	}
}

void MagneticSensors::UpdateMagnetValuesAndPropagate(ChessBoard& board) {
    G8RTOS_AcquireSemaphore(&sensorSem);

    for (uint8_t file = 0; file < 8; file++) {
        auto fileValues = SenseFile(file);

        for (uint8_t rank = 0; rank < 8; rank++) {
            bool piecePresent = fileValues[rank];

            if (currentMagnetValues[file][rank] != piecePresent) {
                // Detected change!
                currentMagnetValues[file][rank] = piecePresent;

                if (piecePresent) {
                    board.PlacePiece(Cell(file, rank));
                } else {
                    board.LiftPiece(Cell(file, rank));
                }
            }
        }

        G8RTOS_SleepThread(1);
    }

    G8RTOS_ReleaseSemaphore(&sensorSem);
}

decltype(MagneticSensors::currentMagnetValues) MagneticSensors::GetCurrentMagnetValues() const {
    G8RTOS_AcquireSemaphore(&sensorSem);    
    auto ret = currentMagnetValues;
    G8RTOS_ReleaseSemaphore(&sensorSem);

    return ret;
}
