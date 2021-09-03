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

    enum class PlayType { 
        ONLINE, LOCAL
    };

    struct GameState {
        PlayType playType;
        PlayerColor localPlayTurn;
    };

	class Board {
        public:
        enum class BoardState {
              AWAITING_LOCAL_MOVE
            , AWAITING_REMOTE_MOVE_NOTICE
            , AWAITING_REMOTE_MOVE_FOLLOWTHROUGH
            , WON_GAME
            , LOST_GAME
            , LOCAL_WHITE_WIN
            , LOCAL_BLACK_WIN
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
            void t_WinLocalWhite();
            void t_WinLocalBlack();

            bool CanMakeLocalMove() const;
            bool CanFollowthroughRemoteMove() const;
            bool CanReceiveRemoteMove() const;
        };

        mutable Semaphore boardSem = { 1 };

        FSM fsm;
		LedMatrix ledMatrix;
        MagneticSensors magneticSensors;
        Brightness brightness = Brightness::MAX;

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

        // Promotion information
        bool isPerformingPromotion = false;
        PromotionType promotionType = PromotionType::NONE;
        bool liftedPromotion = false;
        bool placedPromotion = false;

        // Game over information
        RemoteChess::optional<Cell> winningKingPos;
        RemoteChess::optional<Cell> losingKingPos;

        RemoteChess::flat_unordered_set<Cell, 64> invalidLifts; // Make sure you have your daily dose of squatz and oatz
        RemoteChess::flat_unordered_set<Cell, 64> invalidPlacements;

        RemoteChess::optional<Move> lastRemoteMove{ Move(Cell(1, 7), Cell(2, 5)) };
        RemoteChess::optional<Move> lastLocalMove;

        std::array<std::array<RemoteChess::flat_vector<MoveFragment, 32>, 8>, 8> allLegalMoves = {}; // allLegalMoves[file][rank]

		public:

		Board(PlayerColor color, BoardState initialState);

        void LiftPiece(const Cell& location);
        void PlacePiece(const Cell& location);

        RemoteChess::optional<Move> SubmitCurrentLocalMove(bool isLocalGame = false);
        bool IsPotentialLocalMoveValid() const;
        bool ReceiveRemoteMove(const Move& move, bool inCheckmate = false);

        RemoteChess::optional<Move> GetLastMove() const;
        bool IsInCheck() const { return checkKingPos.HasValue(); };

        void UpdateLedMatrix();
        void UpdateMagneticSensors();

        void SetCheck(const Cell& kingPos);
        void ClearCheck();

        bool MustSelectPromotion() const;
        void SetPromotionType(PromotionType promType);
        RemoteChess::optional<PromotionType> GetPromotionInProgress() const;

        void SetUpcomingCheckmate(const Cell& winningKingPos, const Cell& losingKingPos);

        void WinGame();
        void WinGame(const Cell& winningKingPos, const Cell& losingKingPos);
        void LoseGame();
        void WinLocal(PlayerColor winner, const Cell& winningKingPos, const Cell& losingKingPos);
        void WinLocal(PlayerColor winner);

        void GoToIdle();
        void SetBrightness(Brightness brightness);

        void UpdateLegalMoves();
        GameState UpdateFromServer();

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
