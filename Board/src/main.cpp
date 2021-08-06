extern "C" {
	#include "msp.h"
	#include "BSP.h"
	#include <stdio.h>
	#include "G8RTOS_CriticalSection.h"
	#include "G8RTOS.h"
	#include "chessServer.h"
}

#include <src/ChessBoard.h>
#include "MagneticSensors.h"
#include "LCD_CharacterDisplay.h"
#include "ButtonInterface.h"

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
	while (true) {
		g_board.UpdateMagneticSensors();

		G8RTOS_SleepThread(20);
	}

	// G8RTOS_SleepThread(1000);
	// g_board.LiftPiece(Cell(1, 1));
	// G8RTOS_SleepThread(1000);
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
	// g_board.PlacePiece(Cell(1, 3));
	// G8RTOS_SleepThread(1000);
	// g_board.LiftPiece(Cell(5, 1));
	// G8RTOS_SleepThread(1000);
	// g_board.PlacePiece(Cell(5, 1));

	// G8RTOS_SleepThread(1000);
	// g_board.SubmitCurrentLocalMove();

	// G8RTOS_SleepThread(1000);
	// g_board.ReceiveRemoteMove(Move(Cell(4, 6), Cell(4, 4)));

	// G8RTOS_SleepThread(1000);
	// g_board.LiftPiece(Cell(4, 6));

	// G8RTOS_SleepThread(1000);
	// g_board.PlacePiece(Cell(4, 4));

	// G8RTOS_KillSelf();
}

void BoardLedUpdateThread() {
	while (true) {
		g_board.UpdateLedMatrix();

		G8RTOS_SleepThread(20);
	}
}

void LCD_Thread() {
	G8RTOS_SleepThread(50);

	LCD_CharacterDisplay lcd;

	lcd.EnableBacklight();
	lcd.WriteMessage({ "This is line 1!", "Here lies line 2", "Hello from line 3!", "Where is line 4?" });
	
	ButtonInterface buttons;

	while (true) {
		ButtonState btnState = buttons.GetCurrentButtonState();

		lcd.WriteLine(btnState.up ? "UP" : "        ", 0);
		lcd.WriteLine(btnState.down ? "DOWN" : "        ", 1);
		lcd.WriteLine(btnState.left ? "LEFT" : "        ", 2);

		if (btnState.right && btnState.center)
			lcd.WriteLine("RIGHT  CENTER", 3);
		else if (btnState.right)
			lcd.WriteLine("RIGHT             ", 3);
		else if (btnState.center)
			lcd.WriteLine("       CENTER     ", 3);
		else
			lcd.WriteLine("                  ", 3);

		G8RTOS_SleepThread(50);
	}
}

void main(void) {
	BSP_InitBoard();

	printf("Hello, world!\r\n");

	printf("Starting wifi...\r\n");
	chessServer_init(KEEP_CONNECTION);

	G8RTOS_Init();

	// G8RTOS_AddThread(FlashThread, 4, "Flash");
	G8RTOS_AddThread(MagnetThread, 2, "MagnetThread");
	G8RTOS_AddThread(BoardLedUpdateThread, 4, "LedThread");
	G8RTOS_AddThread(LCD_Thread, 5, "LCDThread");
	G8RTOS_AddThread(IdleThread, 255, "Idle");


	G8RTOS_Launch();

	// FlashThread();

	while(true) { }
}
