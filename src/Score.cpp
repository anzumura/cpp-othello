#include "Score.h"

namespace othello {

using B = Board;
using Set = const B::Set&;

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

template<int DEC, int INC> inline bool emptyCorner(Set empty, int x, int pos) {
  return (x == 1 && empty[pos - DEC]) || (x == B::RowSub2 && empty[pos + INC]);
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
inline auto emptyUp(Set empty, int pos) {
  return emptyEdge<-B::RowAdd1, -B::Rows, -B::RowSub1, 0, B::RowSub1, B::RowSub1, B::RowAdd1>(empty, pos);
}
inline auto emptyDown(Set empty, int pos) {
  return emptyEdge<B::RowSub1, B::Rows, B::RowAdd1, B::SizeSubRows, -B::RowAdd1, B::SizeSub1, -B::RowSub1>(empty, pos);
}

} // namespace

int Score::scoreBoard(const Board& board, Set myVals, Set opVals) {
  if (board.hasValidMoves()) {
    const B::Set empty = (myVals | opVals).flip();
    int result = 0;
    for (int row = 0, pos = 0; row < B::Rows; ++row)
      for (int col = 0; col < B::Rows; ++col, ++pos)
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
  if (const bool sideEdge = col == 0 || col == B::RowSub1; row == 0 || row == B::RowSub1) {
    const int rowStart = pos - col;
    return sideEdge                                                    ? Corner
      : safeEdge<1>(pos, rowStart, rowStart + B::Rows, myVals, opVals) ? SafeEdge
      : emptyCorner<1, 1>(empty, col, pos)                             ? EmptyCornerEdge
                                                                       : Edge;
  } else if (sideEdge)
    return safeEdge<B::Rows>(pos, 0, B::Size, myVals, opVals) ? SafeEdge
      : emptyCorner<B::Rows, B::Rows>(empty, row, pos)        ? EmptyCornerEdge
                                                              : Edge;
  // process non-edges
  return (row == 1)       ? (emptyCorner<B::RowAdd1, -B::RowSub1>(empty, col, pos) ? EmptyCorner
                               : emptyUp(empty, pos)                               ? EmptyEdge
                                                                                   : CenterEdge)
    : (row == B::RowSub2) ? (emptyCorner<-B::RowSub1, B::RowAdd1>(empty, col, pos) ? EmptyCorner
                               : emptyDown(empty, pos)                             ? EmptyEdge
                                                                                   : CenterEdge)
    : (col == 1)          ? (emptySide<-B::RowAdd1, -1, B::RowSub1>(empty, pos) ? EmptyEdge : CenterEdge)
    : (col == B::RowSub2) ? (emptySide<-B::RowSub1, 1, B::RowAdd1>(empty, pos) ? EmptyEdge : CenterEdge)
                          : Center;
}

} // namespace othello
