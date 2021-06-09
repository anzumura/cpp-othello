#ifndef OTHELLO_SCORE_H
#define OTHELLO_SCORE_H

#include <othello/Board.h>

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
  enum Values { Win = 1'000'000 };
  int score(const Board& board, Board::Color c, bool debugPrint = false) const {
    if (c == Board::Color::Black) return scoreBoard(board, board.black(), board.white(), debugPrint);
    return scoreBoard(board, board.white(), board.black(), debugPrint);
  }
  virtual std::string toString() const = 0;

private:
  // return a score for the given board which could be Win, -Win (loss), 0 (for a draw) or a total of cell scores
  virtual int scoreBoard(const Board& board, const Board::Set& myVals, const Board::Set& opVals,
                         bool debugPrint) const {
    if (board.hasValidMoves()) {
      const Board::Set empty = (myVals | opVals).flip();
      if (debugPrint) return printScoreCells(myVals, opVals, empty);
      return scoreCells(myVals, opVals, empty);
    }
    const int myCount = myVals.count(), opCount = opVals.count();
    return myCount > opCount ? Win : myCount < opCount ? -Win : 0;
  }

  // loop through each cell and calculate the aggregate score:
  //   Add to total if cell contains my color
  //   Subtract from total if cell contains opposite color
  int scoreCells(const Board::Set& myVals, const Board::Set& opVals, const Board::Set& empty) const {
    int result = 0;
    for (int row = 0, pos = 0; row < Board::Rows; ++row)
      for (int col = 0; col < Board::Rows; ++col, ++pos)
        if (myVals[pos])
          result += scoreCell(row, col, pos, myVals, opVals, empty);
        else if (opVals[pos])
          result -= scoreCell(row, col, pos, opVals, myVals, empty);
    return result;
  }

  // print the score of each cell in a grid to help testing:
  //   Scores for opposite color are inside ()
  //   Empty cells show '....' to help distinguish them from a '0' calculated score
  //   Prints totals for my color and oppsite color below the grid
  //   Asserts total calculated matchs result from calling non-debug version of 'scoreCells'
  int printScoreCells(const Board::Set&, const Board::Set&, const Board::Set&) const;

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
  //   BadEdge: edge 'bad' location since the adjacent corner is empty
  //   Edge: edge location that isn't one of the locations already mentioned
  //   BadCenter: non-edge 'bad' location since the adjacent corner is empty, i.e.: b2, g2, b6, g6
  //   Bad: non-edge 'bad' location since at least one of the adjacent edges is empty
  //   CenterEdge: adjacent-to-edge location that isn't one of the locations already mentioned
  //   Center: location not in the outer two rows or columns
  enum Values {
    BadEdge = -8,
    BadCenter = -10,
    Bad = -4,
    CenterEdge = 0,
    Center = 1,
    Edge = 4,
    SafeEdge = 12,
    Corner = 16
  };
  std::string toString() const override { return "FullScore"; }

private:
  int scoreCell(int, int, int, const Board::Set&, const Board::Set&, const Board::Set&) const override;
};

class WeightedScore : public Score {
public:
  std::string toString() const override { return "WeightedScore"; }
  // Meanings are similar to FullScore, but since there is no functionality for 'checking for empty or safe'
  // less overall values are needed to populate 'WeightedScoreValues' matrix (see Score.cpp for more details)
  enum Values { BadCenter = -4, BadEdge, Bad = -1, CenterEdge = 0, Center, Edge, Corner = 4 };

private:
  int scoreCell(int, int, int, const Board::Set&, const Board::Set&, const Board::Set&) const override;
};

} // namespace othello

#endif // OTHELLO_SCORE_H
