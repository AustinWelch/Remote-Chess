#include "Move.h"

using namespace RemoteChess;

Move::Move(const char algabreic[6]) : from(Cell(algabreic)), to(Cell(algabreic + 2)), isAttackingMove(false), moveType(MoveType::NORMAL) {
    char special = *(algabreic + 4);

    if (special == 'A')
        isAttackingMove = true;
    else if (special == 'Q')
        moveType = MoveType::QUEENSIDE_CASTLE;
    else if (special == 'K')
        moveType = MoveType::KINGSIDE_CASTLE;
    else if (special == 'E')
        moveType = MoveType::EN_PASSANT;
}

std::array<char, 5> Move::GetAlgabreic() const {
    return { from.file + 0x61, from.rank + 0x31, to.file + 0x61, to.rank + 0x31, '\0' };
}

Move Move::GetRookCastleMove(const Move& kingMove) {
    Cell from, to;
    auto rank = kingMove.from.rank;

    if (kingMove.moveType == MoveType::KINGSIDE_CASTLE) {
        from = Cell(7, rank);
        to = Cell(5, rank);
    } else if (kingMove.moveType == MoveType::QUEENSIDE_CASTLE) {
        from = Cell(0, rank);
        to = Cell(3, rank);
    }

    return Move(from, to);
}

bool Move::operator==(const Move& rhs) const {
    return from == rhs.from && to == rhs.to && isAttackingMove == rhs.isAttackingMove && moveType == rhs.moveType;
}

