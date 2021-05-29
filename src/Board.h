#ifndef OTHELLO_BOARD_H
#define OTHELLO_BOARD_H

#include <bitset>
#include <iostream>
#include <string>
#include <vector>

namespace othello {

enum Values {
  FirstPos = 0, OneColumn, RowSizeMinusTwo = 6, RowSizeMinusOne, RowSize, RowSizePlusOne,
  SecondRowEnd = 15, SeventhRowStart = 48, BoardSize = 64
};

class Board {
 public:
  enum class Color : int8_t { Black, White };

  // construct a Board with initial '4 disk' position
  Board() : black((1LL << 28) | (1LL << 35)), white((1LL << 27) | (1LL << 36)) {}

  // construct a Board from a 64 length char representation where:
  //   . = empty cell
  //   x = black cell
  //   o = white cell
  explicit Board(const char*);

  // simple toString function to help with testing (returns a 64 length string)
  std::string toString() const;

  // get current piece counts
  auto blackCount() const { return black.count(); }
  auto whiteCount() const { return white.count(); }

  // get list of valid moves for a given color
  std::vector<std::string> validMoves(Color) const;

  // Set updates Board to reflect the new position (including performing flips)
  // and returns the number of disks that were flipped. If no disks would be
  // flipped by the move then Board is not be updated (since the move is
  // illegal). pos should be a value like 'a1' or 'h8' (column letter followed
  // by row number)
  int set(const char* pos, Color);
 private:
  friend std::ostream& operator<<(std::ostream&, const Board&);
  static auto posToString(int pos) {
    std::string result(1, 'a' + pos % RowSize);
    result.push_back('1' + pos / RowSize);
    return result;
  }

  bool occupied(int pos) const { return black.test(pos) || white.test(pos); }
  using Set = std::bitset<BoardSize>;
  bool validMove(int pos, const Set& myValues, const Set& opValues) const;
  int set(int pos, Set& myValues, Set& opValues);

  Set black;
  Set white;
};

// output friendly printing including borders with letters and numbers
std::ostream& operator<<(std::ostream&, const Board&);

}  // namespace othello

#endif  // OTHELLO_BOARD_H
