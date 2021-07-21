#pragma once

namespace RemoteChess {
	struct Cell {
		uint8_t file;
		uint8_t rank;

		Cell(uint8_t file, uint8_t rank) : file(file), rank(rank) { }
        Cell() : Cell(0, 0) { }

        bool operator==(const Cell& rhs) const { return file == rhs.file && rank == rhs.rank; };
        bool operator!=(const Cell& rhs) const { return !(*this == rhs); };

        struct Hash {
            std::size_t operator()(const Cell& cell) const {
                return std::hash<uint8_t>()(cell.file) ^ std::hash<uint8_t>()(cell.rank);
            }
        };
	};
}
