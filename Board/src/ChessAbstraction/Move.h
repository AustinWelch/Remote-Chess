#pragma once

#include "Cell.h"
#include <array>

namespace RemoteChess {
    enum class MoveType {
          NORMAL
        , QUEENSIDE_CASTLE
        , KINGSIDE_CASTLE
        , EN_PASSANT
    };

    struct MoveFragment {
        Cell to;
        bool isAttackingMove;
        MoveType moveType;

        MoveFragment(const Cell& to, bool attacking = false, MoveType moveType = MoveType::NORMAL)
         : to(to), isAttackingMove(attacking), moveType(moveType) { };
        MoveFragment() : MoveFragment(Cell()) { }
    };

    struct Move {
        Cell from;
        Cell to;
        bool isAttackingMove;
        MoveType moveType;
        

        Move(const Cell& from, const Cell& to, bool attacking = false, MoveType moveType = MoveType::NORMAL)
         : from(from), to(to), isAttackingMove(attacking), moveType(moveType) { };
        Move(const Cell& from, const MoveFragment& frag) : Move(from, frag.to, frag.isAttackingMove, frag.moveType) { };
        Move(const char algabreic[6]);

        std::array<char, 5> GetAlgabreic() const;

        static Move GetRookCastleMove(const Move& kingMove);

        bool operator==(const Move& rhs) const;
    };
}
