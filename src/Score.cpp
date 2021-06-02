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

int Score::score(const Board& board, int row, int col, const Board::Set& myValues, const Board::Set& opValues) {
  if (isCorner(row, col)) return Corner;
  if (isSafeEdge(row, col)) return SafeEdge;
  if (isEdge(row, col)) return Edge;
  if (isCenter(row, col)) return Center;
  if (isNextToEdge(row, col)) return NextToEdge;
  return NextToCorner;
}

}  // namespace othello