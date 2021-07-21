extern "C"
{
#include "chessServer.h"
}
#include <map>
#include <string>

using namespace std;

map<int, flat_vector<Cell, 32>> allLegalMoves;
map<int, flat_vector<Cell, 32>> allAttackingMoves;
map<int, string> pieceNames;

RemoteChess::flat_vector<Cell, 32> Board::GetLegalMovesPiece(const Cell &origin) const
{
	flat_vector<Cell, 32> legalMoves;

	int pos = (origin.rank + origin.file * 8);

	flat_vector<Cell, 32> &cells = &allLegalMoves.at(pos);

	for (const Cell &cell : cells)
		legalMoves.push_back(cell);

	return legalMoves;
}

RemoteChess::flat_vector<Cell, 32> Board::GetAttackMovesPiece(const Cell &origin) const
{
	flat_vector<Cell, 32> attackingMoves;

	int pos = (origin.rank + origin.file * 8);

	flat_vector<Cell, 32> &cells = &allAttackingMoves.at(pos);

	for (const Cell &cell : cells)
		attackingMoves.push_back(cell);

	return attackingMoves;
}

RemoteChess::string Board::GetPieceName(const Cell &cell) const
{
	int pos = (cell.rank + cell.file * 8);
	return pieceNames.at(pos);
}

void Board::GetLegalMovesAll(const Cell &origin) const
{
	allLegalMoves.erase(allLegalMoves.begin(), allLegalMoves.end());
	pieceNames.erase(pieceNames.begin(), pieceNames.end());

	string movesString[1024];
	chessServer_getLegalMoves(movesString);

	char *strpt = movesString + 12;

	int curPos;
	string name[6];
	while (*strpt != '}')
	{
		curPos = (*(strpt + 1) - 97) + ((*(strpt + 2) - 49) * 8);
		strpt += 8;

		name.clear();
		int i = 0;
		while (*strpt != '\'')
			name[i++] = *strpt++;

		pieceNames.insert(pos, name);
		strpt += 3;

		flat_vector<Cell, 32> legalMovesPiece;
		flat_vector<Cell, 32> attackingMovesPiece;
		while (*strpt != ']')
		{
			Cell curCell = Cell(*(strpt + 1) - 96, *(strpt + 2));
			legalMovesPiece.push_back(curCell);
			if (*(strpt + 3) != '\'')
			{
				if (*(strpt + 3) == 'A')
					attackingMovesPiece.push_back(curCell);
				//TODO: Handle castling
				strpt++;
			}
			strpt += 4;
			if (*strpt == ',')
				strpt += 2;
		}

		allLegalMoves.insert(pos, legalMovesPiece);
		allAttackingMoves.insert(pos, attackingMovesPiece);

		strpt++;
		if (*strpt == ',')
			strpt += 2;
	}
}
