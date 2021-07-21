extern "C" {
	#include "msp.h"
	#include "BSP.h"
	#include <stdio.h>
	#include "G8RTOS_CriticalSection.h"
	#include "G8RTOS.h"
}

#include "Board.h"

using namespace RemoteChess;

RemoteChess::Board g_board(PlayerColor::WHITE);

void IdleThread() {
	while (true) { }
}

void FlashThread() {
	while (true) {
		// board.ledMatrix.DrawChecker();
		// board.ledMatrix.Refresh();

		// // for (volatile int i = 0; i < 2000000; i++) { };

		// // board.ledMatrix.DrawChecker();
		// // board.ledMatrix.Refresh();

		// for (volatile int i = 0; i < 2000000; i++) { };

		// board->ledMatrix.SetAll(Colors::BLUE);
		// board->ledMatrix.Refresh();

		// for (volatile int i = 0; i < 2000000; i++) { };
	}
}

void MagnetThread() {
	G8RTOS_SleepThread(1000);
	g_board.LiftPiece(Cell(1, 1));
	G8RTOS_SleepThread(1000);
	// g_board.PlacePiece(Cell(1, 1));
	// G8RTOS_SleepThread(1000);
	// g_board.LiftPiece(Cell(1, 1));
	// G8RTOS_SleepThread(1000);
	// g_board.PlacePiece(Cell(4, 2));
	// G8RTOS_SleepThread(1000);
	// g_board.LiftPiece(Cell(4, 2));
	// G8RTOS_SleepThread(1000);
	// g_board.PlacePiece(Cell(1, 3));
	// G8RTOS_SleepThread(1000);
	// g_board.LiftPiece(Cell(1, 3));
	// G8RTOS_SleepThread(1000);
	g_board.PlacePiece(Cell(1, 3));
	// G8RTOS_SleepThread(1000);
	// g_board.LiftPiece(Cell(5, 1));
	// G8RTOS_SleepThread(1000);
	// g_board.PlacePiece(Cell(5, 1));

	G8RTOS_SleepThread(1000);
	g_board.SubmitCurrentLocalMove();

	G8RTOS_SleepThread(1000);
	g_board.ReceiveRemoteMove(Move(Cell(4, 6), Cell(4, 4)));

	G8RTOS_SleepThread(1000);
	g_board.LiftPiece(Cell(4, 6));

	G8RTOS_SleepThread(1000);
	g_board.PlacePiece(Cell(4, 4));
	
	G8RTOS_KillSelf();
}

void BoardLedUpdateThread() {
	while (true) {
		g_board.UpdateLedMatrix();

		G8RTOS_SleepThread(100);
	}
}

void main(void) {
	BSP_InitBoard();

	printf("Hello, world!\r\n");

	G8RTOS_Init();

	// G8RTOS_AddThread(FlashThread, 4, "Flash");
	G8RTOS_AddThread(MagnetThread, 2, "MagnetThread");
	G8RTOS_AddThread(BoardLedUpdateThread, 4, "LedThread");
	G8RTOS_AddThread(IdleThread, 255, "Idle");

    P3->DIR = BIT0; // Make pin 0 and output
	P3->OUT = 1; // Data line is defaulted to low (inverted in external circuit)

	G8RTOS_Launch();

	// FlashThread();

	while(true) { }
}
