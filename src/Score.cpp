#include "Score.h"

namespace othello {

namespace {

template<int INC>
inline bool checkSafe(int pos, int low, int high, const Board::Set& myValues, const Board::Set& opValues) {
  bool allMyValue = true;
  int i = pos + INC;
  for (; i < high; i += INC)
    if (opValues.test(i)) allMyValue = false;
    else if (!myValues.test(i)) break;
  if (allMyValue) {
    if (i >= high) return true;  // all cells contain myValue to right (or bottom) corner
    for (i = pos - INC; i >= low; i -= INC)
      if (opValues.test(i)) allMyValue = false;
      else if (!myValues.test(i)) return false;
    return allMyValue; // all cells contain myValue to left (or top) corner
  } else if (i >= high)
    for (i = pos - INC; i >= low; i -= INC)
      if (!opValues.test(i) && !myValues.test(i)) return false;
  return i < low; // entire row is occupied so consider safe
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
        if (myValues.test(pos)) result += scoreCell(board, row, col, pos, myValues, opValues);
        else if (opValues.test(pos)) result -= scoreCell(board, row, col, pos, myValues, opValues);
    return result;
  }
  int myCount = myValues.count();
  int opCount = opValues.count();
  return myCount > opCount ? Win : myCount < opCount ? -Win : 0;
}

int Score::scoreCell(const Board& board, int row, int col, int pos, const Board::Set& myValues,
 const Board::Set& opValues) {
  const bool isTopBottom = row == 0 || row == 7;
  const bool isLeftRight = col == 0 || col == 7;
  if (isTopBottom) {
    const int rowStart = pos - col;
    return isLeftRight ? Corner : checkSafe<1>(pos, rowStart, rowStart + Board::RowSize,
     myValues, opValues) ? SafeEdge : Edge;
  }
  if (isLeftRight)
    return (checkSafe<Board::RowSize>(pos, 0, Board::BoardSize, myValues, opValues)) ? SafeEdge : Edge;
  if (isCenter(row, col)) return Center;
  if (isNextToEdge(row, col)) return NextToEdge;
  return NextToCorner;
}

}  // namespace othello