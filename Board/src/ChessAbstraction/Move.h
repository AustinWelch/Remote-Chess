#pragma once

#include "Cell.h"

namespace RemoteChess {
    struct Move {
        Cell from;
        Cell to;

        Move(const Cell& from, const Cell& to) : from(from), to(to) { }
    };
}
