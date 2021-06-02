#ifndef OTHELLO_SCORE_H
#define OTHELLO_SCORE_H

#include "Board.h"

namespace othello {

class Score {
 public:
  enum Values {
    NextToCorner = -16, NextToEdge = -4,
    Center = 1, Edge = 16, SafeEdge = 64, Corner = 2048,
    Win = 1'000'000
  };
  static int score(const Board& board, Board::Color c) {
    if (c == Board::Color::Black)
      return score(board, board.black(), board.white());
    return score(board, board.white(), board.black());
  }
 private:
  static int score(const Board& board, const Board::Set& myValues, const Board::Set& opValues) {
    int result = 0, pos = 0;
    for (int row = 0; row < Board::RowSize; ++row)
      for (int col = 0; col < Board::RowSize; ++col, ++pos)
        if (myValues.test(pos)) result += score(board, row, col, myValues, opValues);
        else if (opValues.test(pos)) result -= score(board, row, col, myValues, opValues);
    return result;
  }
  static int score(const Board&, int, int, const Board::Set&, const Board::Set&);
};

}  // namespace othello

#endif  // OTHELLO_SCORE_H
