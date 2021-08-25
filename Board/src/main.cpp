#include <src/ChessBoard.h>
#include "MagneticSensors.h"
#include "LCD_CharacterDisplay.h"
#include "ButtonInterface.h"
#include "FSM.h"

extern "C" {
	#include "msp.h"
	#include "BSP.h"
	#include <stdio.h>
	#include "G8RTOS_CriticalSection.h"
	#include "G8RTOS.h"
	#include "chessServer.h"
}

using namespace RemoteChess;

RemoteChess::Board g_board(PlayerColor::WHITE, Board::BoardState::NO_GAME);
RemoteChess::FSM g_fsm(FSM::State::INITIAL_CONNECTION);

void IdleThread() {
	while (true) { }
}

void MagnetThread() {
	while (true) {
		g_board.UpdateMagneticSensors();

		G8RTOS_SleepThread(20);
	}
}

void BoardLedUpdateThread() {
	while (true) {
		g_board.UpdateLedMatrix();

		G8RTOS_SleepThread(20);
	}
}

void ControlThread(void) {
	g_fsm.FSMController();
}

void main(void) {
	BSP_InitBoard();

	G8RTOS_Init();

	G8RTOS_AddThread(MagnetThread, 2, "MagnetThread");
	G8RTOS_AddThread(BoardLedUpdateThread, 4, "LedThread");
	G8RTOS_AddThread(ControlThread, 7, "ControlThread");
	G8RTOS_AddThread(IdleThread, 255, "Idle");

	G8RTOS_Launch();

	while(true) { }
}
