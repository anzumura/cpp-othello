#include "Score.h"

#include <iomanip>

namespace othello {

using B = Board;
using Set = const B::Set&;

int Score::printScoreCells(Set myVals, Set opVals, Set empty) const {
  int myScore = 0, opScore = 0;
  for (int row = 0, pos = 0; row < B::Rows; ++row) {
    for (int col = 0; col < B::Rows; ++col, ++pos)
      if (myVals[pos]) {
        int s = scoreCell(row, col, pos, myVals, opVals, empty);
        myScore += s;
        std::cout << std::setw(8) << s << " ";
      } else if (opVals[pos]) {
        int s = scoreCell(row, col, pos, opVals, myVals, empty);
        opScore += s;
        std::cout << "   (" << std::setw(4) << s << ")";
      } else
        std::cout << "    .... ";
    std::cout << std::endl;
  }
  std::cout << "Score: " << myScore << " - (" << opScore << ") = " << myScore - opScore << std::endl;
  // make sure the score calculated in this function matches the 'non-debug' scoreCells function
  // will need to remove this assertion if a non-deterministic version of 'scoreCell' is created
  assert(scoreCells(myVals, opVals, empty) == myScore - opScore);
  return myScore - opScore;
}

namespace {

// see comments in Score.h for definition of a 'SafeEdge' position
template<int INC> inline bool safeEdge(int pos, int low, int high, Set myVals, Set opVals, Set empty) {
  bool allMine = true;
  int i = pos + INC;
  // check to the right (or down) and break if we hit a space
  for (; i < high; i += INC)
    if (opVals[i])
      allMine = false;
    else if (!myVals[i])
      break;
  // didn't hit a space to the right
  if (i >= high) {
    if (allMine) return true; // all my color to the right
    // there were opposite colors to the right so check left
    for (i = pos - INC; i >= low; i -= INC)
      if (empty[i]) return false; // hit a space so not safe
    return true;                  // return safe since entire row is occupied
  }
  // hit a space to the right so check left
  for (i = pos - INC; i >= low; i -= INC)
    if (opVals[i] || !myVals[i]) return false; // hit an opposite value or a space so return false
  return true;                                 // all my color to the left
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
  // special cases for positions diagonally next to corners (check 4 edges, but
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

// Static weights from 'An Analysis of Heuristics in Othello'
using W = WeightedScore;
constexpr std::array Score1 = {W::Corner, W::BadEdge, W::Edge, W::Edge, W::Edge, W::Edge, W::BadEdge, W::Corner};
constexpr std::array Score2 = {W::BadEdge, W::BadCenter, W::Bad, W::Bad, W::Bad, W::Bad, W::BadCenter, W::BadEdge};
constexpr std::array Score3 = {W::Edge, W::Bad, W::Center, W::CenterEdge, W::CenterEdge, W::Center, W::Bad, W::Edge};
constexpr std::array Score4 = {W::Edge, W::Bad, W::CenterEdge, W::Center, W::Center, W::CenterEdge, W::Bad, W::Edge};
constexpr std::array WeightedScoreValues = {Score1, Score2, Score3, Score4, Score4, Score3, Score2, Score1};

} // namespace

int FullScore::scoreCell(int row, int col, int pos, Set myVals, Set opVals, Set empty) const {
  // process edges
  if (const bool sideEdge = col == 0 || col == B::RowSub1; row == 0 || row == B::RowSub1) {
    const int rowStart = pos - col;
    return sideEdge                                                           ? Corner
      : safeEdge<1>(pos, rowStart, rowStart + B::Rows, myVals, opVals, empty) ? SafeEdge
      : emptyCorner<1, 1>(empty, col, pos)                                    ? BadEdge
                                                                              : Edge;
  } else if (sideEdge)
    return safeEdge<B::Rows>(pos, 0, B::Size, myVals, opVals, empty) ? SafeEdge
      : emptyCorner<B::Rows, B::Rows>(empty, row, pos)               ? BadEdge
                                                                     : Edge;
  // process non-edges
  return (row == 1)       ? (emptyCorner<B::RowAdd1, -B::RowSub1>(empty, col, pos) ? BadCenter
                               : emptyUp(empty, pos)                               ? Bad
                                                                                   : CenterEdge)
    : (row == B::RowSub2) ? (emptyCorner<-B::RowSub1, B::RowAdd1>(empty, col, pos) ? BadCenter
                               : emptyDown(empty, pos)                             ? Bad
                                                                                   : CenterEdge)
    : (col == 1)          ? (emptySide<-B::RowAdd1, -1, B::RowSub1>(empty, pos) ? Bad : CenterEdge)
    : (col == B::RowSub2) ? (emptySide<-B::RowSub1, 1, B::RowAdd1>(empty, pos) ? Bad : CenterEdge)
                          : Center;
}

int WeightedScore::scoreCell(int row, int col, int, Set, Set, Set) const { return WeightedScoreValues[row][col]; }

} // namespace othello
