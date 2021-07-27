#include "Board.h"
#include <algorithm>
#include <utility>

using namespace RemoteChess;
using namespace std;

Board::Board(PlayerColor color) : fsm(color == PlayerColor::WHITE ? BoardState::AWAITING_LOCAL_MOVE : BoardState::AWAITING_REMOTE_MOVE_NOTICE) {
    chessServer_init(CLOSE_CONNECTION);
}

void Board::LiftPiece(const Cell& cell) {
	G8RTOS_AcquireSemaphore(&boardSem);

	if (fsm.CanMakeLocalMove()) {
		if (cell == placedPiece) {
			// Check for if lifting up a piece that was previously legally placed before confirming move with button
			placedPiece = nullptr;
		} else if (invalidPlacements.contains(cell)) {
			// Lifting up a piece that was placed invalidly
			invalidPlacements.erase(cell);
		} else if (CanLiftPiece(cell) && liftedPiece == nullptr) {
			// Check that the piece is legally moveable and we haven't already lifted another piece
			liftedPiece = cell;
		} else {
			// Lifted up a piece we weren't allowed to move (such as an enemy piece not takeable or a piece blocking check)
			invalidLifts.insert(cell);
		}
	} else if (fsm.CanFollowthroughRemoteMove() && lastRemoteMove.HasValue() && cell == lastRemoteMove->from) {
		// Check if making remote move
		liftedPiece = cell;
	} else {
		invalidLifts.insert(cell);
	}

	G8RTOS_ReleaseSemaphore(&boardSem);
}

void Board::PlacePiece(const Cell& cell) {
	G8RTOS_AcquireSemaphore(&boardSem);

	if (liftedPiece == nullptr) {
		// Unexpected piece placed without lifting anything
		invalidPlacements.insert(cell);
	} else {
		if (fsm.CanMakeLocalMove()) {
			if (cell == liftedPiece) {
				// Placing a lifted piece back where it started
				liftedPiece = nullptr;
			} else {
				auto legalMoves = GetLegalMovesPiece(*liftedPiece);

				if (legalMoves.contains(cell)) {
					// Placed a valid move for the lifted piece
					placedPiece = cell;
				} else if (invalidLifts.contains(cell)) {
					// Placed a piece back down when it was illegally lifted
					invalidLifts.erase(cell);
				} else {
					// Placed an illegal move for the lifted piece
					invalidPlacements.insert(cell);
				}
			}
		} else if (fsm.CanFollowthroughRemoteMove()) {
			if (cell == lastRemoteMove->to) {
				placedPiece = cell;

				CompleteRemoteMoveFollowthrough();
			} else {
				invalidPlacements.insert(cell);
			}
		} else {
			invalidPlacements.insert(cell);
		}
	}

	G8RTOS_ReleaseSemaphore(&boardSem);
}

RemoteChess::flat_vector<Cell, 32> Board::GetLegalMovesPiece(const Cell& origin) const {
	int pos = ((origin.rank - 1) * 8 + (origin.file - 1));

	return allLegalMoves[pos];
}

RemoteChess::flat_vector<Cell, 8> Board::GetAttackingMovesPiece(const Cell& origin) const {
    int pos = ((origin.rank - 1) * 8 + (origin.file - 1));
	return allAttackingMoves[pos];
}

std::string Board::GetPieceName(const Cell& cell) const {
    int pos = ((cell.rank - 1) * 8 + (cell.file - 1));
	return pieceNames[pos];
}

void Board::GetLegalMovesAll() {
//	allLegalMoves.erase_all();
//	allAttackingMoves.erase_all();
//	pieceNames.erase_all();

	char movesString[1024];

	G8RTOS_StartCriticalSection();
	chessServer_getLegalMoves(movesString);
	G8RTOS_EndCriticalSection();

	printf(movesString);

	char *strpt = movesString + 14;

	int curPos;
	while(*strpt != '}')
	{
		curPos = (*(strpt+1) - 97) + ((*(strpt+2) - 49) * 8);
		strpt += 8;
	
		std::string &name = pieceNames[curPos];
		name.clear();
		int i = 0;
		while(*strpt != '\'')
		    name[i++] = *strpt++;

		strpt += 3;
		
		flat_vector<Cell, 32> legalMovesPiece;
		flat_vector<Cell, 8> attackingMovesPiece;
		while(*strpt != ']') {
			Cell curCell = Cell(*(strpt + 1) - 96,*(strpt + 2)-48);
			legalMovesPiece.push_back(curCell);
			if (*(strpt + 3) != '\'') {
				if (*(strpt + 3) == 'A')
					attackingMovesPiece.push_back(curCell);
				//TODO: Handle castling
				strpt++;
			}
			strpt += 4;
			if(*strpt == ',')
				strpt += 2;
		}

		allLegalMoves[curPos] = legalMovesPiece;
		allAttackingMoves[curPos] = attackingMovesPiece;

		strpt++;
		if(*strpt == ',')
			strpt += 2;
	}
}

bool Board::CanLiftPiece(const Cell& cell) const {
	return true;
}

void Board::SubmitCurrentLocalMove() {
	G8RTOS_AcquireSemaphore(&boardSem);

	if (!fsm.CanMakeLocalMove())
		return;

	if (!invalidLifts.is_empty() || !invalidPlacements.is_empty())
		return;

	if (!liftedPiece.HasValue() || !placedPiece.HasValue())
		return;

	lastLocalMove = Move(liftedPiece.Value(), placedPiece.Value());
	lastRemoteMove = nullptr;

	liftedPiece = nullptr;
	placedPiece = nullptr;

	fsm.t_LocalMoveSubmitted();

	G8RTOS_ReleaseSemaphore(&boardSem);
}

void Board::CompleteRemoteMoveFollowthrough() {
	if (!fsm.CanFollowthroughRemoteMove())
		return;

	liftedPiece = nullptr;
	placedPiece = nullptr;

	fsm.t_RemoteMoveFollowthroughed();
}

void Board::ReceiveRemoteMove(const Move& move) {
	G8RTOS_AcquireSemaphore(&boardSem);

	if (!fsm.CanReceiveRemoteMove())
		return;

	lastRemoteMove = move;
	lastLocalMove = nullptr;

	fsm.t_RemoteMoveReceived();

	G8RTOS_ReleaseSemaphore(&boardSem);
}

void Board::DrawRemoteMove() {
	if (lastRemoteMove.HasValue()) {
		ledMatrix.SetCell(lastRemoteMove->from, Colors::MAGENTA);
		ledMatrix.SetCell(lastRemoteMove->to, Colors::MAGENTA);
	}
}

void Board::UpdateLedMatrix() {
	G8RTOS_AcquireSemaphore(&boardSem);

	ledMatrix.DrawChecker();

	switch (fsm.GetState()) {
		case BoardState::AWAITING_LOCAL_MOVE:
			DrawRemoteMove();

			if (liftedPiece) {
				ledMatrix.SetCell(*liftedPiece, Colors::GREEN);

				if (placedPiece) {
					ledMatrix.SetCell(*placedPiece, Colors::GREEN);
				} else {
					ledMatrix.SetCells(GetLegalMovesPiece(*liftedPiece), Colors::BLUE);
				}
			}

			break;
		case BoardState::AWAITING_REMOTE_MOVE_NOTICE:
			if (lastLocalMove) {
				ledMatrix.SetCell(lastLocalMove->from, Colors::MAGENTA);
				ledMatrix.SetCell(lastLocalMove->to, Colors::MAGENTA);
			}

			break;
		case BoardState::AWAITING_REMOTE_MOVE_FOLLOWTHROUGH:
			if (!liftedPiece) {
				ledMatrix.SetCell(lastRemoteMove->from, Colors::MAGENTA);
				ledMatrix.SetCell(lastRemoteMove->to, Colors::MAGENTA);
			} else {
				ledMatrix.SetCell(lastRemoteMove->to, Colors::BLUE);
			}
		default:
			break;
	}

	for (const Cell& cell : invalidPlacements) {
		ledMatrix.SetCell(cell, Colors::RED);
	}

	for (const Cell& cell : invalidLifts) {
		ledMatrix.SetCell(cell, Colors::RED);
	}

	ledMatrix.Refresh();

	G8RTOS_ReleaseSemaphore(&boardSem);
}

Board::BoardState Board::FSM::GetState() const {
	BoardState retVal = curState;

	return retVal;
}

void Board::FSM::t_LocalMoveSubmitted() {
	if (CanMakeLocalMove()) {
		curState = BoardState::AWAITING_REMOTE_MOVE_NOTICE;
	}
}

void Board::FSM::t_RemoteMoveFollowthroughed() {
	if (CanFollowthroughRemoteMove()) {
		curState = BoardState::AWAITING_LOCAL_MOVE;
	}
}

void Board::FSM::t_RemoteMoveReceived() {
	if (CanReceiveRemoteMove()) {
		curState = BoardState::AWAITING_REMOTE_MOVE_FOLLOWTHROUGH;
	}
}

bool Board::FSM::CanMakeLocalMove() const {
	return curState == BoardState::AWAITING_LOCAL_MOVE;
}

bool Board::FSM::CanFollowthroughRemoteMove() const {
	return curState == BoardState::AWAITING_REMOTE_MOVE_FOLLOWTHROUGH;
}

bool Board::FSM::CanReceiveRemoteMove() const {
	return curState == BoardState::AWAITING_REMOTE_MOVE_NOTICE;
}
