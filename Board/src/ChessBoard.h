#pragma once

#include <cstdio>
#include "LedMatrix.h"
#include "MagneticSensors.h"
#include "optional.h"
#include "flat_vector.h"
#include "flat_unordered_set.h"
#include "Move.h"
#include <unordered_set>

extern "C" {
    #include "G8RTOS_Semaphores.h"
}

namespace RemoteChess {
    enum class PlayerColor {
        WHITE, BLACK
    };

	class Board {
        public:
        enum class BoardState {
              AWAITING_LOCAL_MOVE
            , AWAITING_REMOTE_MOVE_NOTICE
            , AWAITING_REMOTE_MOVE_FOLLOWTHROUGH
            , WON_GAME
            , LOST_GAME
            , NO_GAME
        };

        private:
        class FSM {
            BoardState curState;

            public:
            FSM(BoardState initialState) : curState(initialState) { }
            BoardState GetState() const;

            void t_LocalMoveSubmitted();
            void t_RemoteMoveFollowthroughed();
            void t_RemoteMoveReceived();
            void t_Win();
            void t_Lose();

            bool CanMakeLocalMove() const;
            bool CanFollowthroughRemoteMove() const;
            bool CanReceiveRemoteMove() const;
        };

        mutable Semaphore boardSem = { 1 };

        FSM fsm;
		LedMatrix ledMatrix;
        MagneticSensors magneticSensors;

        bool inCheckmate = false;

        RemoteChess::optional<Cell> attackedPiece;
        RemoteChess::optional<Cell> liftedPiece;
        RemoteChess::optional<Cell> placedPiece;

        RemoteChess::optional<Cell> checkKingPos;

        // Castling information
        RemoteChess::optional<Move> currentRookCastleMove;
        bool liftedCastle = false;
        bool placedCastle = false;

        // En Passant
        RemoteChess::optional<Cell> enPassantCapture;
        bool liftedEnPassant = false;

        // Game over information
        RemoteChess::optional<Cell> winningKingPos;
        RemoteChess::optional<Cell> losingKingPos;

        RemoteChess::flat_unordered_set<Cell, 64> invalidLifts; // Make sure you have your daily dose of squatz and oatz
        RemoteChess::flat_unordered_set<Cell, 64> invalidPlacements;

        RemoteChess::optional<Move> lastRemoteMove{ Move(Cell(1, 7), Cell(2, 5)) };
        RemoteChess::optional<Move> lastLocalMove;

        std::array<std::array<RemoteChess::flat_vector<MoveFragment, 32>, 8>, 8> allLegalMoves = {}; // allLegalMoves[file][rank]
        // RemoteChess::flat_vector<std::string, 32> pieceNames = {};

		public:

		Board(PlayerColor color, BoardState initialState);

        void LiftPiece(const Cell& location);
        void PlacePiece(const Cell& location);

        RemoteChess::optional<Move> SubmitCurrentLocalMove();
        bool IsPotentialLocalMoveValid() const;
        bool ReceiveRemoteMove(const Move& move, bool inCheckmate = false);

        RemoteChess::optional<Move> GetLastMove() const;
        bool IsInCheck() const { return checkKingPos.HasValue(); };

        void UpdateLedMatrix();
        void UpdateMagneticSensors();

        void SetCheck(const Cell& kingPos);
        void ClearCheck();

        void SetUpcomingCheckmate(const Cell& winningKingPos, const Cell& losingKingPos);

        void WinGame();
        void WinGame(const Cell& winningKingPos, const Cell& losingKingPos);
        void LoseGame();
        void GoToIdle();

        void UpdateLegalMoves();
        void UpdateFromServer();

        BoardState GetCurrentState() const;

        private:
        void HighlightPieceYellow(const Cell& cell);
        void DrawRemoteMove();
        void CompleteRemoteMoveFollowthrough();
        const RemoteChess::flat_vector<MoveFragment, 32>& GetLegalMoves(const Cell& origin) const;
        RemoteChess::optional<Move> GetLegalMove(const Cell& origin, const Cell& dest) const;
        bool CanLiftPiece(const Cell& origin) const;
	};
}
