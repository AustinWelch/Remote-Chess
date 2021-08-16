#include <src/ChessBoard.h>
#include <algorithm>
#include "msp.h"

extern "C" {
	#include "chessServer.h"
	#include "G8RTOS_Scheduler.h"
}

using namespace RemoteChess;

Board::Board(PlayerColor color, BoardState initalState) : fsm(initalState) {

}

void Board::LiftPiece(const Cell& cell) {
	G8RTOS_AcquireSemaphore(&boardSem);

	if (invalidPlacements.contains(cell)) {
		// Lifting up a piece that was placed invalidly
		invalidPlacements.erase(cell);
	} else if (fsm.GetState() == BoardState::NO_GAME) {
		invalidLifts.insert(cell);
	} else if (fsm.CanMakeLocalMove()) {
		if (cell == placedPiece) {
			// Check for if lifting up a piece that was previously legally placed before confirming move with button
			placedPiece = nullptr;

			// Cancel the previous castling attempt
			currentRookCastleMove = nullptr;

			// Cancel the previous en passant attempt
			enPassantCapture = nullptr;
		} else if (CanLiftPiece(cell)) {
			// Check that the piece is legally moveable and we haven't already lifted another piece
			if (liftedPiece == nullptr) {
				const auto& legalMoves = GetLegalMoves(cell);

				if (legalMoves.size() != 0)
					liftedPiece = cell;
				else
					invalidLifts.insert(cell);
			} else if (currentRookCastleMove != nullptr) {
				// We are castling
				if (cell == currentRookCastleMove->from)
					liftedCastle = true;
				else if (cell == currentRookCastleMove->to)
					placedCastle = false;
				else
					invalidLifts.insert(cell);
			} else if (enPassantCapture && cell == enPassantCapture) {
				liftedEnPassant = true;
			} else {
				// We are not castling
				// Check if we are attacking a piece
				auto potentialAttack = GetLegalMove(*liftedPiece, cell);

				if (potentialAttack.HasValue() && potentialAttack->isAttackingMove) {
					attackedPiece = cell;
				} else {
					invalidLifts.insert(cell);
				}
			}
		} else {
			// Lifted up a piece we weren't allowed to move (such as an enemy piece not takeable or a piece blocking check)
			invalidLifts.insert(cell);
		}
	} else if (fsm.CanFollowthroughRemoteMove() && lastRemoteMove.HasValue()) {
		// Check if making remote move
		if (cell == lastRemoteMove->from) {
			liftedPiece = cell;
 		} else if (cell == lastRemoteMove->to && lastRemoteMove->isAttackingMove) {
			attackedPiece = cell;
		} else if (liftedPiece && placedPiece) {
			if (currentRookCastleMove) {
				if (cell == currentRookCastleMove->from) {
					liftedCastle = true;
				} else if (cell == currentRookCastleMove->to) {
					placedCastle = false;
				} else {
					invalidLifts.insert(cell);
				}
			} else if (enPassantCapture) {
				if (cell == *enPassantCapture) {
					liftedEnPassant = true;
					CompleteRemoteMoveFollowthrough();
				} else {
					invalidLifts.insert(cell);
				}
			}
		} else {
			invalidLifts.insert(cell);
		}
	} else {
		invalidLifts.insert(cell);
	}

	G8RTOS_ReleaseSemaphore(&boardSem);
}

void Board::PlacePiece(const Cell& cell) {
	G8RTOS_AcquireSemaphore(&boardSem);

	if (invalidLifts.contains(cell)) {
		// Placed a piece back down when it was illegally lifted
		invalidLifts.erase(cell);
	} else if (liftedPiece == nullptr && !fsm.CanFollowthroughRemoteMove()) {
		// Unexpected piece placed without lifting anything
		invalidPlacements.insert(cell);
	} else if (fsm.GetState() == BoardState::NO_GAME) {
		invalidPlacements.insert(cell);
	} else {
		if (fsm.CanMakeLocalMove()) {
			if (cell == liftedPiece) {
				// Placing a lifted piece back where it started
				liftedPiece = nullptr;
			} else if (!currentRookCastleMove && !enPassantCapture) {
				// We are not castling, check legal moves
				RemoteChess::optional<Move> legalMove = GetLegalMove(*liftedPiece, cell);

				if (legalMove.HasValue()) {
					// Placed a valid move for the lifted piece
					placedPiece = cell;

					if (legalMove->moveType == MoveType::KINGSIDE_CASTLE || legalMove->moveType == MoveType::QUEENSIDE_CASTLE) {
						currentRookCastleMove = Move::GetRookCastleMove(*legalMove);
					} else if (legalMove->moveType == MoveType::EN_PASSANT) {
						enPassantCapture = lastRemoteMove->to;
					}
				} else {
					// Placed an illegal move for the lifted piece
					invalidPlacements.insert(cell);
				}
			} else if (currentRookCastleMove) {
				if (liftedCastle) {
					// We are castling
					if (cell == currentRookCastleMove->to)
						placedCastle = true;
					else if (cell == currentRookCastleMove->from)
						liftedCastle = false;
					else
						invalidPlacements.insert(cell);
				} else {
					invalidPlacements.insert(cell);
				}
			} else if (enPassantCapture) {
				if (liftedEnPassant && cell == *enPassantCapture) {
					liftedEnPassant = false;
				} else {
					invalidPlacements.insert(cell);
				}
			}
		} else if (fsm.CanFollowthroughRemoteMove()) {
			if (cell == lastRemoteMove->to) {
				// Check for if we are attacking a piece
				if (!lastRemoteMove->isAttackingMove) {
					if (liftedPiece) {
						placedPiece = cell;

						if (!currentRookCastleMove && !enPassantCapture)
							CompleteRemoteMoveFollowthrough();
					} else {
						invalidPlacements.insert(cell);
					}
				} else if (attackedPiece.HasValue()) {
					if (liftedPiece){
						placedPiece = cell;

						if (!currentRookCastleMove && !enPassantCapture)
							CompleteRemoteMoveFollowthrough();
					} else {
						attackedPiece = nullptr;
					}
				} else {
					invalidPlacements.insert(cell);
				}
			} else if (cell == lastRemoteMove->from) {
				liftedPiece = nullptr;
			} else if (liftedPiece && placedPiece && currentRookCastleMove) {
				// Check for castling
				if (cell == currentRookCastleMove->to) {
					// Completed castle, now complete move
					placedCastle = true;
					CompleteRemoteMoveFollowthrough();
				} else if (cell == currentRookCastleMove->from) {
					// Placed castling piece back down
					liftedCastle = false;
				} else {
					invalidPlacements.insert(cell);
				}
			} else {
				invalidPlacements.insert(cell);
			}
		} else {
			invalidPlacements.insert(cell);
		}
	}

	G8RTOS_ReleaseSemaphore(&boardSem);
}

const RemoteChess::flat_vector<MoveFragment, 32>& Board::GetLegalMoves(const Cell& origin) const {
	return allLegalMoves[origin.file][origin.rank];
}

RemoteChess::optional<Move> Board::GetLegalMove(const Cell& origin, const Cell& dest) const {
	const auto& legalMoves = GetLegalMoves(origin);

	for (const MoveFragment& legalDest : legalMoves) {
		if (dest == legalDest.to)
			return Move(origin, legalDest);
	}

	return nullptr;
}

void Board::SetCheck(const Cell& kingPos) {
	checkKingPos = kingPos;
}

void Board::ClearCheck() {
	checkKingPos = nullptr;
}

void Board::UpdateLegalMoves() {
	G8RTOS_AcquireSemaphore(&boardSem);

	for (auto& rankMoves : allLegalMoves) {
		for (auto& pieceMoves : rankMoves) {
			pieceMoves.clear();
		}
	}

	static char legalMovesString[1024];
	chessServer_getLegalMoves(legalMovesString);

	const char* strpt = legalMovesString + 14;

	int curPos;
	while (*strpt != '}')
	{
		Cell originCell(strpt + 1);

		strpt += 8;
	
		// std::string &name = pieceNames[curPos];
		// name.clear();
		// int i = 0;
		while (*strpt != '\'')
			strpt++;
		    // name[i++] = *strpt++;

		strpt += 3;
		
		// flat_vector<Cell, 32> legalMoves
		// flat_vector<Cell, 8> attackingMovesPiece;

		while (*strpt != ']') {
			MoveFragment foundLegalMove(Cell(strpt + 1));

			if (*(strpt + 3) != '\'') {
				char special = *(strpt + 3);
				if (special == 'A')
					foundLegalMove.isAttackingMove = true;
				else if (special == 'K')
					foundLegalMove.moveType = MoveType::KINGSIDE_CASTLE;
				else if (special == 'Q')
					foundLegalMove.moveType = MoveType::QUEENSIDE_CASTLE;
				else if (special == 'E')
					foundLegalMove.moveType = MoveType::EN_PASSANT;

				strpt++;
			}

			allLegalMoves[originCell.file][originCell.rank].push_back(foundLegalMove);

			strpt += 4;
			if(*strpt == ',')
				strpt += 2;
		}

		// allLegalMoves[curPos] = legalMovesPiece;
		// allAttackingMoves[curPos] = attackingMovesPiece;

		strpt++;
		if(*strpt == ',')
			strpt += 2;
	}

	G8RTOS_ReleaseSemaphore(&boardSem);
}

void Board::UpdateFromServer() {
	G8RTOS_AcquireSemaphore(&boardSem);

	ServerGameState gameState = chessServer_getGameState();

	RemoteChess::optional<Move> lastMove;

	liftedPiece = nullptr;
	placedPiece = nullptr;
	attackedPiece = nullptr;
	invalidLifts.clear();
	invalidPlacements.clear();
	lastRemoteMove = nullptr;
	lastLocalMove = nullptr;

	currentRookCastleMove = nullptr;
	liftedCastle = false;
	placedCastle = false;

	enPassantCapture = nullptr;
	liftedEnPassant = false;

	ClearCheck();
	winningKingPos = nullptr;
	checkKingPos = nullptr;
	inCheckmate = false;

	if (gameState.status == SUCCESS || gameState.status == SERVER_LOST_GAME) {
		if (gameState.hasLastMove)
			lastMove = Move(gameState.algabreicLastMove);

		if (gameState.activePlayer == LOCAL_MOVE) { 
			lastRemoteMove = lastMove;

			if (lastRemoteMove.HasValue()) {
				// It's our turn, check if we still need to follow through
				// Every move in chess will leave the originating square empty.
				if (magneticSensors.GetCurrentMagnetValues()[lastRemoteMove->from.file][lastRemoteMove->from.rank] == false) {
					// If empty, we already followed through, await local move
					fsm = FSM(BoardState::AWAITING_LOCAL_MOVE);
				} else {
					fsm = FSM(BoardState::AWAITING_REMOTE_MOVE_FOLLOWTHROUGH);

					if (lastRemoteMove->moveType == MoveType::KINGSIDE_CASTLE || lastRemoteMove->moveType == MoveType::QUEENSIDE_CASTLE)
						currentRookCastleMove = Move::GetRookCastleMove(*lastRemoteMove);
					else if (lastRemoteMove->moveType == MoveType::EN_PASSANT) {
						enPassantCapture = lastRemoteMove->to;

						if (enPassantCapture->rank == 2)
							enPassantCapture->rank++;
						else if (enPassantCapture->rank == 5)
							enPassantCapture->rank--;
					}
				}
			} else {
				fsm = FSM(BoardState::AWAITING_LOCAL_MOVE);
			}
		} else {
			lastLocalMove = lastMove;
			fsm = FSM(BoardState::AWAITING_REMOTE_MOVE_NOTICE);
		}

		if (gameState.inCheck) {
			SetCheck(Cell(gameState.algabreicKingPosCheck));
		} else {
			ClearCheck();
		}

		if (gameState.status == SERVER_LOST_GAME) {
			SetUpcomingCheckmate(Cell(gameState.algabreicKingPosWinner), Cell(gameState.algabreicKingPosCheck));

			if (fsm.GetState() == BoardState::AWAITING_LOCAL_MOVE) {
				LoseGame();
			}
		}
	} else if (gameState.status == SERVER_WON_GAME) {
		fsm = FSM(BoardState::WON_GAME);

		WinGame(Cell(gameState.algabreicKingPosWinner), Cell(gameState.algabreicKingPosCheck));
	}

	magneticSensors.MeasureInitialMagnetValues();

	G8RTOS_ReleaseSemaphore(&boardSem);

	if (!inCheckmate) {
		UpdateLegalMoves();
	}
}

bool Board::CanLiftPiece(const Cell& cell) const {
	return true;
}

RemoteChess::optional<Move> Board::SubmitCurrentLocalMove() {
	G8RTOS_AcquireSemaphore(&boardSem);

	if (!IsPotentialLocalMoveValid()) {
		G8RTOS_ReleaseSemaphore(&boardSem);
		return nullptr;
	}

	lastLocalMove = Move(liftedPiece.Value(), placedPiece.Value());
	lastRemoteMove = nullptr;

	liftedPiece = nullptr;
	placedPiece = nullptr;
	ClearCheck();

	currentRookCastleMove = nullptr;
	liftedCastle = false;
	placedCastle = false;

	enPassantCapture = nullptr;
	liftedEnPassant = false;

	fsm.t_LocalMoveSubmitted();

	G8RTOS_ReleaseSemaphore(&boardSem);

	return lastLocalMove;
}

bool Board::IsPotentialLocalMoveValid() const {
	return     fsm.CanMakeLocalMove()
			&& invalidLifts.is_empty() && invalidPlacements.is_empty()
			&& liftedPiece.HasValue() && placedPiece.HasValue()
			&& (!currentRookCastleMove || (liftedCastle && placedCastle))
			&& (!enPassantCapture || liftedEnPassant);
}

void Board::CompleteRemoteMoveFollowthrough() {
	if (!fsm.CanFollowthroughRemoteMove())
		return;

	liftedPiece = nullptr;
	placedPiece = nullptr;
	attackedPiece = nullptr;

	currentRookCastleMove = nullptr;
	liftedCastle = false;
	placedCastle = false;

	enPassantCapture = nullptr;
	liftedEnPassant = false;

	if (!inCheckmate)
		fsm.t_RemoteMoveFollowthroughed();
	else
		fsm.t_Lose();
}

bool Board::ReceiveRemoteMove(const Move& move, bool inCheckmate) {
	G8RTOS_AcquireSemaphore(&boardSem);

	if (!fsm.CanReceiveRemoteMove()) {
		G8RTOS_ReleaseSemaphore(&boardSem);
		return false;
	}

	lastRemoteMove = move;
	attackedPiece = nullptr;

	currentRookCastleMove = nullptr;
	liftedCastle = false;
	placedCastle = false;

	enPassantCapture = nullptr;
	liftedEnPassant = false;

	if (move.moveType == MoveType::KINGSIDE_CASTLE || move.moveType == MoveType::QUEENSIDE_CASTLE)
		currentRookCastleMove = Move::GetRookCastleMove(move);
	else if (move.moveType == MoveType::EN_PASSANT)
		enPassantCapture = lastLocalMove->to;


	lastLocalMove = nullptr;

	fsm.t_RemoteMoveReceived();

	G8RTOS_ReleaseSemaphore(&boardSem);

	return true;
}

void Board::SetUpcomingCheckmate(const Cell& winningKingPos, const Cell& losingKingPos) {
	this->winningKingPos = winningKingPos;
	this->losingKingPos = losingKingPos;

	inCheckmate = true;
}

void Board::WinGame() {
	fsm.t_Win();
}

void Board::WinGame(const Cell& winningKingPos, const Cell& losingKingPos) {
	this->winningKingPos = winningKingPos;
	this->losingKingPos = losingKingPos;

	fsm.t_Win();
}

void Board::LoseGame() {
	fsm.t_Lose();
}

void Board::GoToIdle() {
	fsm = FSM(BoardState::NO_GAME);
}

RemoteChess::optional<Move> Board::GetLastMove() const {
	if (fsm.GetState() == BoardState::AWAITING_LOCAL_MOVE || fsm.GetState() == BoardState::AWAITING_REMOTE_MOVE_FOLLOWTHROUGH)
		return lastRemoteMove;
	else
		return lastLocalMove;
}

void Board::DrawRemoteMove() {
	if (lastRemoteMove.HasValue()) {
		ledMatrix.SetCell(lastRemoteMove->from, Colors::PURPLE);
		ledMatrix.SetCell(lastRemoteMove->to, Colors::PURPLE);
	}
}

void Board::UpdateMagneticSensors() {
	magneticSensors.UpdateMagnetValuesAndPropagate(*this);
}

void Board::UpdateLedMatrix() {
	G8RTOS_AcquireSemaphore(&boardSem);

	ledMatrix.DrawChecker();

	switch (fsm.GetState()) {
		case BoardState::AWAITING_LOCAL_MOVE:
			DrawRemoteMove();

			if (checkKingPos.HasValue() && !placedPiece) {
				ledMatrix.SetCell(*checkKingPos, Colors::ORANGE);
			}

			if (liftedPiece) {
				if (placedPiece) {
					ledMatrix.SetCell(*liftedPiece, Colors::GREEN);
					ledMatrix.SetCell(*placedPiece, Colors::GREEN);

					if (currentRookCastleMove) {
						if (!liftedCastle && !placedCastle) {
							ledMatrix.SetCell(currentRookCastleMove->from, Colors::YELLOW);
							ledMatrix.SetCell(currentRookCastleMove->to, Colors::YELLOW);
						} else if (liftedCastle && placedCastle) {
							ledMatrix.SetCell(currentRookCastleMove->from, Colors::LIGHT_GREEN);
							ledMatrix.SetCell(currentRookCastleMove->to, Colors::LIGHT_GREEN);
						} else if (liftedCastle) {
							ledMatrix.SetCell(currentRookCastleMove->from, Colors::YELLOW);
							ledMatrix.SetCell(currentRookCastleMove->to, Colors::CYAN);
						}
					} else if (enPassantCapture) {
						if (!liftedEnPassant)
							ledMatrix.SetCell(*enPassantCapture, Colors::ORANGE);
					}
				} else {
					if (attackedPiece) {
						ledMatrix.SetCell(*liftedPiece, Colors::GREEN);
						ledMatrix.SetCell(*attackedPiece, Colors::BLUE);
					} else {
						ledMatrix.SetCell(*liftedPiece, Colors::GREEN);
						
						for (const MoveFragment& mf : GetLegalMoves(*liftedPiece)) {
							ledMatrix.SetCell(mf.to, mf.isAttackingMove ? Colors::ORANGE : Colors::BLUE);
						}
					}
				}
			}

			break;
		case BoardState::AWAITING_REMOTE_MOVE_NOTICE:
			if (lastLocalMove) {
				ledMatrix.SetCell(lastLocalMove->from, Colors::PURPLE);
				ledMatrix.SetCell(lastLocalMove->to, Colors::PURPLE);
			}

			break;
		case BoardState::AWAITING_REMOTE_MOVE_FOLLOWTHROUGH:
			if (!lastRemoteMove->isAttackingMove) {
				if (!placedPiece)
					ledMatrix.SetCell(lastRemoteMove->from, Colors::YELLOW);

				ledMatrix.SetCell(lastRemoteMove->to, liftedPiece ? Colors::CYAN : Colors::YELLOW);

				if (liftedPiece && placedPiece) {
					if (currentRookCastleMove) {
						if (!liftedCastle && !placedCastle) {
							ledMatrix.SetCell(currentRookCastleMove->from, Colors::YELLOW);
							ledMatrix.SetCell(currentRookCastleMove->to, Colors::YELLOW);
						} else if (liftedCastle) {
							ledMatrix.SetCell(currentRookCastleMove->from, Colors::YELLOW);
							ledMatrix.SetCell(currentRookCastleMove->to, Colors::CYAN);
						}
					} else if (enPassantCapture) {
						ledMatrix.SetCell(*enPassantCapture, Colors::ORANGE);
					}
				}
			} else {
				if (!liftedPiece && !attackedPiece) {
					ledMatrix.SetCell(lastRemoteMove->from, Colors::YELLOW);
					ledMatrix.SetCell(lastRemoteMove->to, Colors::ORANGE);
				} else if (liftedPiece && attackedPiece) {
					ledMatrix.SetCell(lastRemoteMove->from, Colors::YELLOW);
					ledMatrix.SetCell(lastRemoteMove->to, Colors::CYAN);
				} else if (liftedPiece) {
					ledMatrix.SetCell(lastRemoteMove->from, Colors::YELLOW);
					ledMatrix.SetCell(lastRemoteMove->to, Colors::ORANGE);
				} else if (attackedPiece) {
					ledMatrix.SetCell(lastRemoteMove->from, Colors::YELLOW);
					ledMatrix.SetCell(lastRemoteMove->to, Colors::YELLOW);
				}
			}

			break;
		case BoardState::WON_GAME:
			ledMatrix.DrawChecker(Colors::LIGHT_GREEN);

			if (winningKingPos)
				ledMatrix.SetCell(*winningKingPos, Colors::GREEN);
			
			if (losingKingPos)
				ledMatrix.SetCell(*losingKingPos, Colors::RED);

			break;
		case BoardState::LOST_GAME:
			ledMatrix.DrawChecker(Colors::LIGHT_ORANGE);

			if (inCheckmate) {
				ledMatrix.SetCell(*winningKingPos, Colors::GREEN);
				ledMatrix.SetCell(*losingKingPos, Colors::RED);
			}

			break;
		default:
			break;
	}

	const auto& errorColor = fsm.GetState() == BoardState::NO_GAME ? Colors::CYAN : Colors::RED;

	for (const Cell& cell : invalidPlacements) {
		ledMatrix.SetCell(cell, errorColor);
	}

	for (const Cell& cell : invalidLifts) {
		ledMatrix.SetCell(cell, errorColor);
	}

	ledMatrix.Refresh();

	G8RTOS_ReleaseSemaphore(&boardSem);
}

Board::BoardState Board::GetCurrentState() const {
	G8RTOS_AcquireSemaphore(&boardSem);

	BoardState retVal = fsm.GetState();

	G8RTOS_ReleaseSemaphore(&boardSem);

	return retVal;
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

void Board::FSM::t_Win() {
	curState = BoardState::WON_GAME;
}

void Board::FSM::t_Lose() {
	curState = BoardState::LOST_GAME;
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
