#ifndef OTHELLO_SCORE_H
#define OTHELLO_SCORE_H

#include "Board.h"

namespace othello {

class Score {
public:
  enum Values {
    EmptyCornerEdge = -32,
    EmptyCorner = -16,
    EmptyEdge = -4,
    CenterEdge = 0,
    Center = 1,
    Edge = 16,
    SafeEdge = 64,
    Corner = 2048,
    Win = 1'000'000
  };
  // If there are no valid moves then return 'Win' if the given color has more pieces or '-Win' if the
  // opposite color has more pieces (or zero for a draw). Otherwise, each cell is examined and a total
  // calculated score is returned. The calculated score takes into account things like:
  //   Corner: most valuable location since it can't be flipped
  //   SafeEdge: edge location that can't be flipped, i.e., same color extends to a corner or edge is full
  //   EmptyCornerEdge: edge 'bad' location since the adjacent corner is empty
  //   Edge: edge location that isn't one of the locations already mentioned
  //   EmptyCorner: non-edge 'bad' location since the adjacent corner is empty, i.e.: b2, g2, b6, g6
  //   EmptyEdge: non-edge 'bad' location since at least one of the adjacent edges is empty
  //   CenterEdge: adjacent-to-edge location that isn't one of the locations already mentioned
  //   Center: location not in the outer two rows or columns
  static int score(const Board& board, Board::Color c) {
    if (c == Board::Color::Black) return scoreBoard(board, board.black(), board.white());
    return scoreBoard(board, board.white(), board.black());
  }

private:
  static int scoreBoard(const Board&, const Board::Set&, const Board::Set&);
  static int scoreCell(const Board&, int, int, int, const Board::Set&, const Board::Set&, const Board::Set&);
};

} // namespace othello

#endif // OTHELLO_SCORE_H
