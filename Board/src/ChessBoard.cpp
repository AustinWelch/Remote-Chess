#include "ChessBoard.h"
#include <algorithm>
#include "msp.h"

using namespace RemoteChess;

ChessBoard::ChessBoard() {
    playerColor = PlayerColor::WHITE;
    ChessBoard::boardFSM = BoardFSM();
}

ChessBoard::ChessBoard(PlayerColor color, ChessBoard::BoardState state) {
	playerColor = color;
	ChessBoard::boardFSM = BoardFSM(state);
}

void ChessBoard::LiftPiece(const Cell& cell) {
	G8RTOS_AcquireSemaphore(&boardSem);

	if (invalidPlacements.contains(cell)) {
		// Lifting up a piece that was placed invalidly
		invalidPlacements.erase(cell);
	} else if (boardFSM.CanMakeLocalMove()) {
		if (cell == placedPiece) {
			// Check for if lifting up a piece that was previously legally placed before confirming move with button
			placedPiece = nullptr;
		} else if (CanLiftPiece(cell) && liftedPiece == nullptr) {
			// Check that the piece is legally moveable and we haven't already lifted another piece
			liftedPiece = cell;
		} else {
			// Lifted up a piece we weren't allowed to move (such as an enemy piece not takeable or a piece blocking check)
			invalidLifts.insert(cell);
		}
	} else if (boardFSM.CanFollowthroughRemoteMove() && lastRemoteMove.HasValue() && cell == lastRemoteMove->from) {
		// Check if making remote move
		liftedPiece = cell;
	} else {
		invalidLifts.insert(cell);
	}

	G8RTOS_ReleaseSemaphore(&boardSem);
}

void ChessBoard::PlacePiece(const Cell& cell) {
	G8RTOS_AcquireSemaphore(&boardSem);

	if (invalidLifts.contains(cell)) {
		// Placed a piece back down when it was illegally lifted
		invalidLifts.erase(cell);
	} else if (liftedPiece == nullptr) {
		// Unexpected piece placed without lifting anything
		invalidPlacements.insert(cell);
	} else {
		if (boardFSM.CanMakeLocalMove()) {
			if (cell == liftedPiece) {
				// Placing a lifted piece back where it started
				liftedPiece = nullptr;
			} else {
				auto legalMoves = GetLegalMovesPiece(*liftedPiece);

				if (legalMoves.contains(cell)) {
					// Placed a valid move for the lifted piece
					placedPiece = cell;
				} else {
					// Placed an illegal move for the lifted piece
					invalidPlacements.insert(cell);
				}
			}
		} else if (boardFSM.CanFollowthroughRemoteMove()) {
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

RemoteChess::flat_vector<Cell, 32> ChessBoard::GetLegalMovesPiece(const Cell& origin) const {
	int pos = ((origin.rank) * 8 + (origin.file));

	return allLegalMoves[pos];
}

RemoteChess::flat_vector<Cell, 8> ChessBoard::GetAttackingMovesPiece(const Cell& origin) const {
    int pos = ((origin.rank) * 8 + (origin.file));
	return allAttackingMoves[pos];
}

std::string ChessBoard::GetPieceName(const Cell& cell) const {
    int pos = ((cell.rank) * 8 + (cell.file));
	return pieceNames[pos];
}

void ChessBoard::GetLegalMovesAll() {
	for (flat_vector<Cell, 32>& pieceMoves : allLegalMoves)
		pieceMoves.erase_all();
	for (flat_vector<Cell, 8>& pieceAttackingMoves : allAttackingMoves)
		pieceAttackingMoves.erase_all();

	char movesString[1024];

	G8RTOS_StartCriticalSection();
	chessServer_getLegalMoves(movesString); 
	G8RTOS_EndCriticalSection();

	printf(movesString);

	char *strpt = movesString + 14;

	int curPos;
	while (*strpt != '}')
	{
		curPos = (*(strpt+1) - 97) + ((*(strpt+2) - 49) * 8);
		strpt += 8;
	
		std::string &name = pieceNames[curPos];
		name.clear();
		int i = 0;
		while (*strpt != '\'')
		    name[i++] = *strpt++;

		strpt += 3;
		
		flat_vector<Cell, 32> legalMovesPiece;
		flat_vector<Cell, 8> attackingMovesPiece;
		while ( *strpt != ']' ) {
			Cell curCell = Cell( *(strpt + 1) - 97, *(strpt + 2) - 49 );
			legalMovesPiece.push_back(curCell);
			if ( *(strpt + 3) != '\'' ) {
				if ( *(strpt + 3) == 'A' )
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

bool ChessBoard::CanLiftPiece(const Cell& cell) const {
	return true;
}

void ChessBoard::SubmitCurrentLocalMove() {
	G8RTOS_AcquireSemaphore(&boardSem);

	if (!boardFSM.CanMakeLocalMove())
		return;

	if (!invalidLifts.is_empty() || !invalidPlacements.is_empty())
		return;

	if (!liftedPiece.HasValue() || !placedPiece.HasValue())
		return;

	lastLocalMove = Move(liftedPiece.Value(), placedPiece.Value());
	lastRemoteMove = nullptr;

	liftedPiece = nullptr;
	placedPiece = nullptr;

	boardFSM.t_LocalMoveSubmitted();

	G8RTOS_ReleaseSemaphore(&boardSem);
}

void ChessBoard::CompleteRemoteMoveFollowthrough() {
	if (!boardFSM.CanFollowthroughRemoteMove())
		return;

	liftedPiece = nullptr;
	placedPiece = nullptr;

	boardFSM.t_RemoteMoveFollowthroughed();
}

void ChessBoard::ReceiveRemoteMove(const Move& move) {
	G8RTOS_AcquireSemaphore(&boardSem);

	if (!boardFSM.CanReceiveRemoteMove())
		return;

	lastRemoteMove = move;
	lastLocalMove = nullptr;

	boardFSM.t_RemoteMoveReceived();

	G8RTOS_ReleaseSemaphore(&boardSem);
}

void ChessBoard::DrawRemoteMove() {
	if (lastRemoteMove.HasValue()) {
		ledMatrix.SetCell(lastRemoteMove->from, Colors::MAGENTA);
		ledMatrix.SetCell(lastRemoteMove->to, Colors::MAGENTA);
	}
}

void Board::UpdateMagneticSensors() {
	magneticSensors.UpdateMagnetValuesAndPropagate(*this);
}

void ChessBoard::UpdateLedMatrix() {
	G8RTOS_AcquireSemaphore(&boardSem);

	ledMatrix.DrawChecker();

	switch (boardFSM.GetState()) {
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

			break;
		default:
			break;
	}

	for (const Cell& cell : invalidPlacements) {
		ledMatrix.SetCell(cell, Colors::RED);
	}

	for (const Cell& cell : invalidLifts) {
		ledMatrix.SetCell(cell, Colors::RED);
	}

// magneticSensors.UpdateMagnetValuesAndPropagate(*this);

	// auto magnetValues = magneticSensors.GetCurrentMagnetValues();

	// for (uint8_t file = 0; file < 8; file++) {
	// 	for (uint8_t rank = 0; rank < 8; rank++) {
	// 		if (magnetValues[file][rank])
	// 			ledMatrix.SetCell(Cell(file, rank), Colors::YELLOW);
	// 	}
	// }

	// for (uint8_t muxSel = 0; muxSel < 8; muxSel++) {
	// 	P5->OUT = muxSel;

	// 	G8RTOS_SleepThread(1);

	// 	if (!(P2->IN & BIT7))
	// 		ledMatrix.SetCell(Cell(ConvertMuxSelToFile(muxSel), 0), Colors::YELLOW);
		
	// 	if (!(P2->IN & BIT6))
	// 		ledMatrix.SetCell(Cell(ConvertMuxSelToFile(muxSel), 1), Colors::YELLOW);
		
	// 	if (!(P2->IN & BIT5))
	// 		ledMatrix.SetCell(Cell(ConvertMuxSelToFile(muxSel), 2), Colors::YELLOW);
		
	// 	if (!(P2->IN & BIT4))
	// 		ledMatrix.SetCell(Cell(ConvertMuxSelToFile(muxSel), 3), Colors::YELLOW);
		
	// 	if (!(P2->IN & BIT3))
	// 		ledMatrix.SetCell(Cell(ConvertMuxSelToFile(muxSel), 4), Colors::YELLOW);
		
	// 	if (!(P4->IN & BIT7))
	// 		ledMatrix.SetCell(Cell(ConvertMuxSelToFile(muxSel), 5), Colors::YELLOW);
		
	// 	if (!(P4->IN & BIT6))
	// 		ledMatrix.SetCell(Cell(ConvertMuxSelToFile(muxSel), 6), Colors::YELLOW);
		
	// 	if (!(P4->IN & BIT5))
	// 		ledMatrix.SetCell(Cell(ConvertMuxSelToFile(muxSel), 7), Colors::YELLOW);
	// }
	
	ledMatrix.Refresh();

	G8RTOS_ReleaseSemaphore(&boardSem);
}

ChessBoard::BoardState ChessBoard::BoardFSM::GetState() const {
	BoardState retVal = curState;
	return retVal;
}

std::string ChessBoard::GetLiftedPieceName() const {
	std::string name = GetPieceName(liftedPiece.Value());
	return name;
}

Cell ChessBoard::GetLiftedPiecePos() const {
	Cell retVal = liftedPiece.Value();
	return retVal;
}

Cell ChessBoard::GetPlacedPiecePos() const {
	Cell retVal = placedPiece.Value();
	return retVal;
}

ChessBoard::BoardState ChessBoard::GetBoardState() const {
    return boardFSM.GetState();
}

RemoteChess::flat_unordered_set<Cell, 64> ChessBoard::GetInvalidLifts( ){
    return ChessBoard::invalidLifts;
}

RemoteChess::flat_unordered_set<Cell, 64> ChessBoard::GetInvalidPlacements() {
    return ChessBoard::invalidPlacements;
}

void ChessBoard::BoardFSM::t_LocalMoveSubmitted() {
	if (CanMakeLocalMove()) {
		curState = BoardState::AWAITING_REMOTE_MOVE_NOTICE;
	}
}

void ChessBoard::BoardFSM::t_RemoteMoveFollowthroughed() {
	if (CanFollowthroughRemoteMove()) {
		curState = BoardState::AWAITING_LOCAL_MOVE;
	}
}

void ChessBoard::BoardFSM::t_RemoteMoveReceived() {
	if (CanReceiveRemoteMove()) {
		curState = BoardState::AWAITING_REMOTE_MOVE_FOLLOWTHROUGH;
	}
}

bool ChessBoard::BoardFSM::CanMakeLocalMove() const {
	return curState == BoardState::AWAITING_LOCAL_MOVE;
}

bool ChessBoard::BoardFSM::CanFollowthroughRemoteMove() const {
	return curState == BoardState::AWAITING_REMOTE_MOVE_FOLLOWTHROUGH;
}

bool ChessBoard::BoardFSM::CanReceiveRemoteMove() const {
	return curState == BoardState::AWAITING_REMOTE_MOVE_NOTICE;
}
