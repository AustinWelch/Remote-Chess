#pragma once

#include <cstdio>
#include "LedMatrix.h"
#include "optional.h"
#include "flat_vector.h"
#include "flat_unordered_set.h"
#include "Move.h"
#include <unordered_set>
#include <string>

extern "C" {
    #include "chessServer.h"
    #include "G8RTOS_Semaphores.h"
};

namespace RemoteChess {
    enum class PlayerColor {
        WHITE, BLACK
    };

	class Board {
        enum class BoardState {
              AWAITING_LOCAL_MOVE
            , AWAITING_REMOTE_MOVE_NOTICE
            , AWAITING_REMOTE_MOVE_FOLLOWTHROUGH
        };

        class FSM {
            BoardState curState;

            public:
            FSM(BoardState initialState) : curState(initialState) { }
            BoardState GetState() const;

            void t_LocalMoveSubmitted();
            void t_RemoteMoveFollowthroughed();
            void t_RemoteMoveReceived();

            bool CanMakeLocalMove() const;
            bool CanFollowthroughRemoteMove() const;
            bool CanReceiveRemoteMove() const;
        };

        mutable Semaphore boardSem = { 1 };

        FSM fsm;
		LedMatrix ledMatrix;

        RemoteChess::optional<Cell> liftedPiece;
        RemoteChess::optional<Cell> placedPiece;

        RemoteChess::flat_unordered_set<Cell, 64> invalidLifts; // Make sure you have your daily dose of squatz and oatz
        RemoteChess::flat_unordered_set<Cell, 64> invalidPlacements;

        RemoteChess::optional<Move> lastRemoteMove{ Move(Cell(1, 7), Cell(2, 5)) };
        RemoteChess::optional<Move> lastLocalMove;

        flat_vector<flat_vector<Cell, 32>, 32> allLegalMoves;
        flat_vector<flat_vector<Cell, 8>, 32> allAttackingMoves;
        flat_vector<std::string, 32> pieceNames;

		public:

		Board(PlayerColor color);

        void LiftPiece(const Cell& location);
        void PlacePiece(const Cell& location);
        
        void SubmitCurrentLocalMove();
        void ReceiveRemoteMove(const Move& move);

        void UpdateLedMatrix();

        private:
        void DrawRemoteMove();
        void CompleteRemoteMoveFollowthrough();
        bool CanLiftPiece(const Cell& origin) const;
        
        RemoteChess::flat_vector<Cell, 32> GetLegalMovesPiece(const Cell& origin);
        RemoteChess::flat_vector<Cell, 32> GetAttackingMovesPiece(const Cell& origin);
        void GetLegalMovesAll(const Cell& origin);
        std::string Board::GetPieceName(const Cell& cell) const;
	};
}
