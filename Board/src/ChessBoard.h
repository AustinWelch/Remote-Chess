#pragma once

#include <cstdio>
#include "LedMatrix.h"
#include "MagneticSensors.h"
#include "optional.h"
#include "flat_vector.h"
#include "flat_unordered_set.h"
#include "Move.h"
#include <unordered_set>
#include <string>

extern "C" {
    #include "chessServer.h"
    #include "G8RTOS_Semaphores.h"
    #include "G8RTOS_CriticalSection.h"
};

namespace RemoteChess {
    enum class PlayerColor {
        WHITE, BLACK
    };

	class ChessBoard {
	    public:
        enum class BoardState {
              AWAITING_LOCAL_MOVE
            , AWAITING_REMOTE_MOVE_NOTICE
            , AWAITING_REMOTE_MOVE_FOLLOWTHROUGH
        };

	    private:
        class BoardFSM {
            BoardState curState;

            public:
            BoardFSM() : curState(BoardState::AWAITING_LOCAL_MOVE) {}
            BoardFSM(BoardState initialState) : curState(initialState) { }
            BoardState GetState() const;

            void t_LocalMoveSubmitted();
            void t_RemoteMoveFollowthroughed();
            void t_RemoteMoveReceived();

            bool CanMakeLocalMove() const;
            bool CanFollowthroughRemoteMove() const;
            bool CanReceiveRemoteMove() const;
        };

        mutable Semaphore boardSem = { 1 };

        BoardFSM boardFSM;
		LedMatrix ledMatrix;
        PlayerColor playerColor;
        MagneticSensors magneticSensors;

        RemoteChess::optional<Cell> liftedPiece;
        RemoteChess::optional<Cell> placedPiece;
        std::string liftedPieceName;

        RemoteChess::flat_unordered_set<Cell, 64> invalidLifts; // Make sure you have your daily dose of squatz and oatz
        RemoteChess::flat_unordered_set<Cell, 64> invalidPlacements;

        RemoteChess::optional<Move> lastRemoteMove;
        RemoteChess::optional<Move> lastLocalMove;

        flat_vector<flat_vector<Cell, 32>, 64> allLegalMoves = {};
        flat_vector<flat_vector<Cell, 8>, 64> allAttackingMoves = {};
        flat_vector<std::string, 32> pieceNames = {};


		public:
        ChessBoard();
		ChessBoard(PlayerColor color, BoardState state);

        void LiftPiece(const Cell& location);
        void PlacePiece(const Cell& location);

        std::string GetLiftedPieceName() const;
        Cell GetLiftedPiecePos() const;
        Cell GetPlacedPiecePos() const;
        
        RemoteChess::flat_unordered_set<Cell, 64> GetInvalidLifts();
        RemoteChess::flat_unordered_set<Cell, 64> GetInvalidPlacements();

        BoardState GetBoardState() const;

        void SubmitCurrentLocalMove();
        void ReceiveRemoteMove(const Move& move);

        void UpdateLedMatrix();
        void UpdateMagneticSensors();

        RemoteChess::flat_vector<Cell, 32> GetLegalMovesPiece(const Cell& origin) const;
        RemoteChess::flat_vector<Cell, 8> GetAttackingMovesPiece(const Cell& origin) const;
        std::string GetPieceName(const Cell& cell) const;
        void GetLegalMovesAll();

        private:
        void HighlightPieceYellow(const Cell& cell);
        void DrawRemoteMove();
        void CompleteRemoteMoveFollowthrough();
        bool CanLiftPiece(const Cell& origin) const;
        
	};
}
