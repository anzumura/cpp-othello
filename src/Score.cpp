#include "Score.h"

namespace othello {

using Set = const Board::Set&;

namespace {

// see comments in Score.h for definition of a 'SafeEdge' position
template<int INC>
inline bool safeEdge(int pos, int low, int high, Set myVals, Set opVals) {
  bool allMine = true;
  int i = pos + INC;
  for (; i < high; i += INC)
    if (opVals.test(i)) allMine = false;
    else if (!myVals.test(i)) break;
  if (allMine) {
    if (i >= high) return true;  // all cells contain myValue to right (or bottom) corner
    for (i = pos - INC; i >= low; i -= INC)
      if (opVals.test(i)) allMine = false;
      else if (!myVals.test(i)) return false;
    return allMine;  // all cells contain myValue to left (or top) corner
  } else if (i >= high)
    for (i = pos - INC; i >= low; i -= INC)
      if (!opVals.test(i) && !myVals.test(i)) return false;
  return i < low;  // if entire row is occupied consider safe
}

template<int DEC, int INC>
inline bool nextToEmptyCorner(Set opVals, int x, int pos) {
  return (x == 1 && !opVals.test(pos - DEC)) || (x == Board::RowsMinusTwo && !opVals.test(pos + INC));
}

inline auto nextToEmptyEdge(int row, int col) {
  return false;
}

}  // namespace

int Score::scoreBoard(const Board& board, Set myVals, Set opVals) {
  if (board.hasValidMoves()) {
    int result = 0, pos = 0;
    for (int row = 0; row < Board::Rows; ++row)
      for (int col = 0; col < Board::Rows; ++col, ++pos)
        if (myVals.test(pos)) result += scoreCell(board, row, col, pos, myVals, opVals);
        else if (opVals.test(pos)) result -= scoreCell(board, row, col, pos, opVals, myVals);
    return result;
  }
  int myCount = myVals.count();
  int opCount = opVals.count();
  return myCount > opCount ? Win : myCount < opCount ? -Win : 0;
}

int Score::scoreCell(const Board& board, int row, int col, int pos, Set myVals, Set opVals) {
  const bool sideEdge = col == 0 || col == Board::RowsMinusOne;
  // process edges
  if (row == 0 || row == Board::RowsMinusOne) {
    const int rowStart = pos - col;
    return sideEdge ? Corner : safeEdge<1>(pos, rowStart, rowStart + Board::Rows, myVals, opVals) ? SafeEdge
      : nextToEmptyCorner<1, 1>(opVals, col, pos) ? BesideEmptyCorner : Edge;
  } else if (sideEdge)
    return safeEdge<Board::Rows>(pos, 0, Board::Size, myVals, opVals) ? SafeEdge
     : nextToEmptyCorner<Board::Rows, Board::Rows>(opVals, row, pos) ? BesideEmptyCorner : Edge;
  // process non-edges
  if (row == 1)
    return nextToEmptyCorner<Board::RowsPlusOne, -Board::RowsMinusOne>(opVals, row, pos) ? BesideEmptyCorner : Center;
  if (row == Board::RowsMinusTwo)
    return nextToEmptyCorner<-Board::RowsMinusOne, Board::RowsPlusOne>(opVals, row, pos) ? BesideEmptyCorner : Center;
  return Center;
}

}  // namespace othello
