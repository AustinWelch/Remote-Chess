#include "MagneticSensors.h"
#include "../ChessBoard.h"

extern "C" {
    #include "msp.h"
    #include "G8RTOS_Scheduler.h"
    #include "G8RTOS_CriticalSection.h"
}

using namespace RemoteChess;

MagneticSensors::MagneticSensors() {
	// Port 3, 5-7 select lines
	P3->DIR |= BIT5 | BIT6 | BIT7;
    
    MeasureInitialMagnetValues();
}

uint8_t MagneticSensors::ConvertFileToMuxSel(uint8_t file) {
    uint8_t muxSel;

	switch (file) {
		case 0:
			muxSel = 7;
            break;
		case 1:
			muxSel = 4;
            break;
		case 2:
			muxSel = 5;
            break;
		case 3:
			muxSel = 6;
            break;
		case 4:
			muxSel = 0;
            break;
		case 5:
			muxSel = 1;
            break;
		case 6:
			muxSel = 2;
            break;
		case 7:
			muxSel = 3;
            break;
	}

    muxSel <<= 5; // Shift over to bits 5-7
    return muxSel;
}

void MagneticSensors::SetMuxSel(uint8_t file) {
    uint8_t muxSel = ConvertFileToMuxSel(file);

    P3->OUT = (P3->OUT & 0x1F) | muxSel;
}

std::array<bool, 8> MagneticSensors::SenseFile(uint8_t file) const {
    SetMuxSel(file);

    uint8_t port6 = P6->IN;
    uint8_t port7 = P7->IN;
    uint8_t port8 = P8->IN;
    uint8_t port9 = P9->IN;

    return {
          !(port8 & BIT5) // Rank 0
        , !(port9 & BIT0)
        , !(port8 & BIT4)
        , !(port8 & BIT2) // Rank 3
        , !(port9 & BIT2) // Rank 4
        , !(port6 & BIT2)
        , !(port7 & BIT3)
        , !(port7 & BIT1) // Rank 7
    };
}

void MagneticSensors::MeasureInitialMagnetValues() {
    for (uint8_t file = 0; file < 8; file++) {
        auto fileValues = SenseFile(file); 

        currentMagnetValues[file] = fileValues;
	}
}

void MagneticSensors::UpdateMagnetValuesAndPropagate(Board& board) {
    G8RTOS_AcquireSemaphore(&sensorSem);
    G8RTOS_StartCriticalSection();

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
    }

    G8RTOS_EndCriticalSection();
    G8RTOS_ReleaseSemaphore(&sensorSem);
}

decltype(MagneticSensors::currentMagnetValues) MagneticSensors::GetCurrentMagnetValues() const {
    G8RTOS_AcquireSemaphore(&sensorSem);    
    auto ret = currentMagnetValues;
    G8RTOS_ReleaseSemaphore(&sensorSem);

    return ret;
}
