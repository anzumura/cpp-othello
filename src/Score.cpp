#include "Score.h"

namespace othello {

using Set = const Board::Set&;

namespace {

// see comments in Score.h for definition of a 'SafeEdge' position
template<int INC> inline bool safeEdge(int pos, int low, int high, Set myVals, Set opVals) {
  bool allMine = true;
  int i = pos + INC;
  for (; i < high; i += INC)
    if (opVals[i])
      allMine = false;
    else if (!myVals[i])
      break;
  if (allMine) {
    if (i >= high) return true; // all cells contain myValue to right (or bottom) corner
    for (i = pos - INC; i >= low; i -= INC)
      if (opVals[i])
        allMine = false;
      else if (!myVals[i])
        return false;
    return allMine; // all cells contain myValue to left (or top) corner
  } else if (i >= high)
    for (i = pos - INC; i >= low; i -= INC)
      if (!opVals[i] && !myVals[i]) return false;
  return i < low; // if entire row is occupied consider safe
}

template<int DEC, int INC> inline bool emptyCorner(Set opVals, int x, int pos) {
  return (x == 1 && !opVals[pos - DEC]) || (x == Board::RowMinusTwo && !opVals[pos + INC]);
}

// Return true if any of the edge positions are empty (based on T1, T2 and T3
// offsets from pos). So if pos is in the second row then check 'up and left',
// 'up' and 'up and right'.
template<int T1, int T2, int T3> inline bool emptySide(Set empty, int pos) {
  return empty[pos + T1] || empty[pos + T2] || empty[pos + T3];
}
template<int T1, int T2, int T3, int LOW, int LOW_INC, int HIGH, int HIGH_INC>
inline bool emptyEdge(Set empty, int pos) {
  if (empty[pos + T2]) return true;
  const int x = pos + T1;
  const int y = pos + T3;
  // special cases for positions diagially next to corners (check 4 edges, but
  // can skip checking corner)
  if (x == LOW) return empty[y] || empty[pos - 1] || empty[pos + LOW_INC];
  if (y == HIGH) return empty[x] || empty[pos + 1] || empty[pos + HIGH_INC];
  return empty[x] || empty[y];
}

} // namespace

int Score::scoreBoard(const Board& board, Set myVals, Set opVals) {
  if (board.hasValidMoves()) {
    const Board::Set empty = (myVals | opVals).flip();
    int result = 0, pos = 0;
    for (int row = 0; row < Board::Rows; ++row)
      for (int col = 0; col < Board::Rows; ++col, ++pos)
        if (myVals[pos])
          result += scoreCell(board, row, col, pos, myVals, opVals, empty);
        else if (opVals[pos])
          result -= scoreCell(board, row, col, pos, opVals, myVals, empty);
    return result;
  }
  int myCount = myVals.count();
  int opCount = opVals.count();
  return myCount > opCount ? Win : myCount < opCount ? -Win : 0;
}

int Score::scoreCell(const Board& board, int row, int col, int pos, Set myVals, Set opVals, Set empty) {
  // process edges
  if (const bool sideEdge = col == 0 || col == Board::RowMinusOne; row == 0 || row == Board::RowMinusOne) {
    const int rowStart = pos - col;
    return sideEdge                                                             ? Corner
           : safeEdge<1>(pos, rowStart, rowStart + Board::Rows, myVals, opVals) ? SafeEdge
           : emptyCorner<1, 1>(opVals, col, pos)                                ? EmptyCorner
                                                                                : Edge;
  } else if (sideEdge)
    return safeEdge<Board::Rows>(pos, 0, Board::Size, myVals, opVals) ? SafeEdge
           : emptyCorner<Board::Rows, Board::Rows>(opVals, row, pos)  ? EmptyCorner
                                                                      : Edge;
  // process non-edges
  return (row == 1) ? (emptyCorner<Board::RowPlusOne, -Board::RowMinusOne>(opVals, row, pos) ? EmptyCorner
                       : emptyEdge<-Board::RowPlusOne, -Board::Rows, -Board::RowMinusOne, 0, Board::RowMinusOne,
                                   Board::RowMinusOne, Board::RowPlusOne>(empty, pos)
                           ? EmptyEdge
                           : Center)
         : (row == Board::RowMinusTwo)
             ? (emptyCorner<-Board::RowMinusOne, Board::RowPlusOne>(opVals, row, pos) ? EmptyCorner
                : emptyEdge<Board::RowMinusOne, Board::Rows, Board::RowPlusOne, Board::Size - Board::Rows,
                            -Board::RowPlusOne, Board::Size - 1, -Board::RowMinusOne>(empty, pos)
                    ? EmptyEdge
                    : Center)
         : (col == 1) ? (emptySide<-Board::RowPlusOne, -1, Board::RowMinusOne>(empty, pos) ? EmptyEdge : Center)
         : (col == Board::RowMinusTwo)
             ? (emptySide<-Board::RowMinusOne, 1, Board::RowPlusOne>(empty, pos) ? EmptyEdge : Center)
             : Center;
}

} // namespace othello
