#include "Move.h"

using namespace RemoteChess;

Move::Move(const char algabreic[8]) : from(Cell(algabreic)), to(Cell(algabreic + 2)), isAttackingMove(false), moveType(MoveType::NORMAL) {
    char special = *(algabreic + 4);

    if (special == 'A')
        isAttackingMove = true;
    else if (special == 'Q')
        moveType = MoveType::QUEENSIDE_CASTLE;
    else if (special == 'K')
        moveType = MoveType::KINGSIDE_CASTLE;
    else if (special == 'E')
        moveType = MoveType::EN_PASSANT;
    else if (special == 'P') {
        moveType = MoveType::PROMOTION;

        char promType = *(algabreic + 5);

        if (promType == 'Q')
            promotionType = PromotionType::QUEEN;
        else if (promType == 'N')
            promotionType = PromotionType::KNIGHT;
        else if (promType == 'R')
            promotionType = PromotionType::ROOK;
        else if (promType == 'B')
            promotionType = PromotionType::BISHOP;

        if (*(algabreic + 6) == 'A')
            isAttackingMove = true;
    }
}

std::array<char, 6> Move::GetAlgabreic() const {
    if (moveType != MoveType::PROMOTION)
        return { from.file + 0x61, from.rank + 0x31, to.file + 0x61, to.rank + 0x31, '\0', '\0' };
    else {
        char promType;

        if (promotionType == PromotionType::QUEEN)
            promType = 'q';
        else if (promotionType == PromotionType::KNIGHT)
            promType = 'n';
        else if (promotionType == PromotionType::ROOK)
            promType = 'r';
        else if (promotionType == PromotionType::BISHOP)
            promType = 'b';

        return { from.file + 0x61, from.rank + 0x31, to.file + 0x61, to.rank + 0x31, promType, '\0' };
    }

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

