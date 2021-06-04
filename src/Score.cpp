#include "Score.h"

namespace othello {

using Set = const Board::Set&;

namespace {

// see comments in Score.h for definition of a 'SafeEdge' position
template<int INC> inline bool safeEdge(int pos, int low, int high, Set myVals, Set opVals) {
  bool allMine = true;
  int i = pos + INC;
  for (; i < high; i += INC)
    if (opVals.test(i))
      allMine = false;
    else if (!myVals.test(i))
      break;
  if (allMine) {
    if (i >= high) return true; // all cells contain myValue to right (or bottom) corner
    for (i = pos - INC; i >= low; i -= INC)
      if (opVals.test(i))
        allMine = false;
      else if (!myVals.test(i))
        return false;
    return allMine; // all cells contain myValue to left (or top) corner
  } else if (i >= high)
    for (i = pos - INC; i >= low; i -= INC)
      if (!opVals.test(i) && !myVals.test(i)) return false;
  return i < low; // if entire row is occupied consider safe
}

template<int DEC, int INC> inline bool besideEmptyCorner(Set opVals, int x, int pos) {
  return (x == 1 && !opVals.test(pos - DEC)) || (x == Board::RowsMinusTwo && !opVals.test(pos + INC));
}

// Return true if any of the edge positions are empty (based on T1, T2 and T3
// offsets from pos). So if pos is in the second row then check 'up and left',
// 'up' and 'up and right'.
template<int T1, int T2, int T3> inline bool besideEmptySide(const Board& board, int pos) {
  return !board.occupied(pos + T1) || !board.occupied(pos + T2) || !board.occupied(pos + T3);
}
template<int T1, int T2, int T3, int LOW, int LOW_INC, int HIGH, int HIGH_INC>
inline bool besideEmptyEdge(const Board& board, int pos) {
  if (board.occupied(pos + T2)) {
    const int x = pos + T1;
    const int y = pos + T3;
    // special cases for positions diagially next to corners (check 4 edges, but
    // can skip checking corner)
    if (x == LOW) return !board.occupied(y) || !board.occupied(pos - 1) || !board.occupied(pos + LOW_INC);
    if (y == HIGH) return !board.occupied(x) || !board.occupied(pos + 1) || !board.occupied(pos + HIGH_INC);
    return !board.occupied(x) || !board.occupied(y);
  }
  return true;
}

} // namespace

int Score::scoreBoard(const Board& board, Set myVals, Set opVals) {
  if (board.hasValidMoves()) {
    int result = 0, pos = 0;
    for (int row = 0; row < Board::Rows; ++row)
      for (int col = 0; col < Board::Rows; ++col, ++pos)
        if (myVals.test(pos))
          result += scoreCell(board, row, col, pos, myVals, opVals);
        else if (opVals.test(pos))
          result -= scoreCell(board, row, col, pos, opVals, myVals);
    return result;
  }
  int myCount = myVals.count();
  int opCount = opVals.count();
  return myCount > opCount ? Win : myCount < opCount ? -Win : 0;
}

int Score::scoreCell(const Board& board, int row, int col, int pos, Set myVals, Set opVals) {
  // process edges
  if (const bool sideEdge = col == 0 || col == Board::RowsMinusOne; row == 0 || row == Board::RowsMinusOne) {
    const int rowStart = pos - col;
    return sideEdge                                                             ? Corner
           : safeEdge<1>(pos, rowStart, rowStart + Board::Rows, myVals, opVals) ? SafeEdge
           : besideEmptyCorner<1, 1>(opVals, col, pos)                          ? BesideEmptyCorner
                                                                                : Edge;
  } else if (sideEdge)
    return safeEdge<Board::Rows>(pos, 0, Board::Size, myVals, opVals)      ? SafeEdge
           : besideEmptyCorner<Board::Rows, Board::Rows>(opVals, row, pos) ? BesideEmptyCorner
                                                                           : Edge;
  // process non-edges
  if (row == 1)
    return besideEmptyCorner<Board::RowsPlusOne, -Board::RowsMinusOne>(opVals, row, pos) ? BesideEmptyCorner
           : besideEmptyEdge<-Board::RowsPlusOne, -Board::Rows, -Board::RowsMinusOne, 0, Board::RowsMinusOne,
                             Board::RowsMinusOne, Board::RowsPlusOne>(board, pos)
               ? BesideEmptyEdge
               : Center;
  if (row == Board::RowsMinusTwo)
    return besideEmptyCorner<-Board::RowsMinusOne, Board::RowsPlusOne>(opVals, row, pos) ? BesideEmptyCorner
           : besideEmptyEdge<Board::RowsMinusOne, Board::Rows, Board::RowsPlusOne, Board::Size - Board::Rows,
                             -Board::RowsPlusOne, Board::Size - 1, -Board::RowsMinusOne>(board, pos)
               ? BesideEmptyEdge
               : Center;
  if (col == 1)
    return besideEmptySide<-Board::RowsPlusOne, -1, Board::RowsMinusOne>(board, pos) ? BesideEmptyEdge : Center;
  if (col == Board::RowsMinusTwo)
    return besideEmptySide<-Board::RowsMinusOne, 1, Board::RowsPlusOne>(board, pos) ? BesideEmptyEdge : Center;
  return Center;
}

} // namespace othello
