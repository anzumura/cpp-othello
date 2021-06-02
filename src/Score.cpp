#include "Score.h"

namespace othello {

namespace {

inline auto isCorner(int row, int col) {
  return row == 0 && (col == 0 || col == 7)
    || row == 7 && (col == 0 || col == 7);
}

inline auto isSafeEdge(int row, int col) {
  return false;
}

inline auto isEdge(int row, int col) {
  return false;
}

inline auto isCenter(int row, int col) {
  return row > 1 && row < 6 && col > 1 && col < 6;
}

inline auto isNextToEdge(int row, int col) {
  return false;
}

}  // namespace

int Score::scoreBoard(const Board& board, const Board::Set& myValues, const Board::Set& opValues) {
  if (board.hasValidMoves()) {
    int result = 0, pos = 0;
    for (int row = 0; row < Board::RowSize; ++row)
      for (int col = 0; col < Board::RowSize; ++col, ++pos)
        if (myValues.test(pos)) result += scoreCell(board, row, col, myValues, opValues);
        else if (opValues.test(pos)) result -= scoreCell(board, row, col, myValues, opValues);
    return result;
  }
  int myCount = myValues.count();
  int opCount = opValues.count();
  return myCount > opCount ? Win : myCount < opCount ? -Win : 0;
}

int Score::scoreCell(const Board& board, int row, int col, const Board::Set& myValues, const Board::Set& opValues) {
  if (isCorner(row, col)) return Corner;
  if (isSafeEdge(row, col)) return SafeEdge;
  if (isEdge(row, col)) return Edge;
  if (isCenter(row, col)) return Center;
  if (isNextToEdge(row, col)) return NextToEdge;
  return NextToCorner;
}

}  // namespace othello