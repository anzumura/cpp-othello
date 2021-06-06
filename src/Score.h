#ifndef OTHELLO_SCORE_H
#define OTHELLO_SCORE_H

#include "Board.h"

namespace othello {

class Score {
public:
  Score() {}
  Score(const Score&) = delete;
  Score& operator=(const Score&) = delete;
  virtual ~Score() = default;

  // If there are no valid moves then return 'Win' if the given color has more pieces or '-Win' if the
  // opposite color has more pieces (or zero for a draw). Otherwise, each cell is examined and a total
  // calculated score is returned.
  enum Values {
    Win = 1'000'000
  };
  int score(const Board& board, Board::Color c, bool debugPrint = false) const {
    if (c == Board::Color::Black) return scoreBoard(board, board.black(), board.white(), debugPrint);
    return scoreBoard(board, board.white(), board.black(), debugPrint);
  }
  virtual std::string toString() const = 0;
private:
  int scoreBoard(const Board&, const Board::Set&, const Board::Set&, bool) const;
  // scoreCell is called for each non-empty cell and is passed the following values:
  //   row: row of the cell (from 0 to 7)
  //   col: column of the cell (from 0 to 7)
  //   pos: the position on the board (from 0 to 63)
  //   myVals: bitset<64> with values set to true for same color positions (as the cell being scored)
  //   opVals: bitset<64> with values set to true for opposite color positions
  //   empty: bitset<64> with values set to true for empty positions
  virtual int scoreCell(int row, int col, int pos, const Board::Set& myVals, const Board::Set& opVals,
   const Board::Set& empty) const = 0;
};

class FullScore : public Score {
public:
  // The score of a cell will be one of the following values:
  //   Corner: most valuable location since it can't be flipped
  //   SafeEdge: edge location that can't be flipped, i.e., same color extends to a corner or edge is full
  //   EmptyCornerEdge: edge 'bad' location since the adjacent corner is empty
  //   Edge: edge location that isn't one of the locations already mentioned
  //   EmptyCorner: non-edge 'bad' location since the adjacent corner is empty, i.e.: b2, g2, b6, g6
  //   EmptyEdge: non-edge 'bad' location since at least one of the adjacent edges is empty
  //   CenterEdge: adjacent-to-edge location that isn't one of the locations already mentioned
  //   Center: location not in the outer two rows or columns
  enum Values {
    EmptyCornerEdge = -32,
    EmptyCorner = -16,
    EmptyEdge = -4,
    CenterEdge = 0,
    Center = 1,
    Edge = 16,
    SafeEdge = 64,
    Corner = 2048
  };
  std::string toString() const override { return "FullScore"; }
private:
  int scoreCell(int, int, int, const Board::Set&, const Board::Set&, const Board::Set&) const override;
};

class WeightedScore : public Score {
public:
  std::string toString() const override { return "WeightedScore"; }
private:
  int scoreCell(int, int, int, const Board::Set&, const Board::Set&, const Board::Set&) const override;
};

} // namespace othello

#endif // OTHELLO_SCORE_H
