
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
#include "FSM.h"

using namespace RemoteChess;

RemoteChess::Board g_board(PlayerColor::WHITE, Board::BoardState::NO_GAME);
RemoteChess::FSM g_fsm(FSM::State::INITIAL_CONNECTION);
// static LCD_CharacterDisplay lcd;

// LCD_CharacterDisplay lcd;

void IdleThread() {
	while (true) { }
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

// void LCD_Thread() {
// 	G8RTOS_SleepThread(50);

// 	lcd.Init();

// 	lcd.EnableBacklight();
// 	lcd.WriteMessage({ "This is line 1!", "Here lies line 2", "Hello from line 3!", "Where is line 4?" });
	
// 	ButtonInterface buttons;

// 	while (true) {
// 		ButtonState btnState = buttons.GetCurrentButtonState();

// 		lcd.WriteLine(btnState.up ? "UP" : "        ", 0);
// 		lcd.WriteLine(btnState.down ? "DOWN" : "        ", 1);
// 		lcd.WriteLine(btnState.left ? "LEFT" : "        ", 2);

// 		if (btnState.right && btnState.center)
// 			lcd.WriteLine("RIGHT  CENTER", 3);
// 		else if (btnState.right)
// 			lcd.WriteLine("RIGHT             ", 3);
// 		else if (btnState.center)
// 			lcd.WriteLine("       CENTER     ", 3);
// 		else
// 			lcd.WriteLine("                  ", 3);

// 		G8RTOS_SleepThread(50);
// 	}
// }

void ControlThread(void) {
	g_fsm.FSMController();
	// ButtonInterface buttons;

	// g_board.UpdateFromServer();

	// G8RTOS_SleepThread(500);

	// while (true) {
	// 	ButtonState btnState = buttons.GetCurrentButtonState();

	// 	if (g_board.GetCurrentState() == Board::BoardState::AWAITING_LOCAL_MOVE) {
	// 		if (btnState.center) {
	// 			G8RTOS_SleepThread(1);
	// 			btnState = buttons.GetCurrentButtonState();

	// 			Debounced
	// 			if (btnState.center) {
	// 				RemoteChess::optional<Move> localMove = g_board.SubmitCurrentLocalMove();

	// 				if (localMove.HasValue()) {
	// 					chessServer_makeMove(localMove->GetAlgabreic().data());
	// 				}
	// 			}
	// 		}

	// 		G8RTOS_SleepThread(10);
	// 	} else if (g_board.GetCurrentState() == Board::BoardState::AWAITING_REMOTE_MOVE_NOTICE) {
	// 		G8RTOS_SetThreadSwitchable(false);
	// 		AwaitingMove awaitingMove = chessServer_awaitTurn();
	// 		G8RTOS_SetThreadSwitchable(true);

	// 		if (awaitingMove.status == SUCCESS) {
	// 			Move remoteMove(awaitingMove.algabreic);

	// 			g_board.ReceiveRemoteMove(remoteMove);

	// 			char legalMovesString[1024];
	// 			chessServer_getLegalMoves(legalMovesString);
	// 			g_board.UpdateLegalMoves();
	// 		}

	// 		G8RTOS_SleepThread(3000);
	// 	} else {
	// 		G8RTOS_SleepThread(10);
	// 	}
	// }
}

void main(void) {
	BSP_InitBoard();

	printf("Hello, world!\r\n");

	printf("Starting wifi...\r\n");

	// chessServer_init(KEEP_CONNECTION);
	// chessServer_setGameCode("451122");

	G8RTOS_Init();

	// G8RTOS_AddThread(FlashThread, 4, "Flash");
	G8RTOS_AddThread(MagnetThread, 2, "MagnetThread");
	G8RTOS_AddThread(BoardLedUpdateThread, 4, "LedThread");
	//G8RTOS_AddThread(LCD_Thread, 10, "LCDThread");
	G8RTOS_AddThread(ControlThread, 7, "ControlThread");
	G8RTOS_AddThread(IdleThread, 255, "Idle");


	G8RTOS_Launch();

	while(true) { }
}
