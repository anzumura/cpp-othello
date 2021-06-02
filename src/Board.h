#ifndef OTHELLO_BOARD_H
#define OTHELLO_BOARD_H

#include <array>
#include <bitset>
#include <iostream>
#include <string>
#include <vector>

namespace othello {

class Board {
 public:
  enum class Color { Black, White };
  static constexpr std::array Colors = { Color::Black, Color::White };
  enum Values {
    BadLength = -4, BadColumn, BadRow, BadCell,  // see 'set' function for details on error conditions
    RowSizeMinusTwo = 6, RowSizeMinusOne, RowSize, RowSizePlusOne, MaxValidMoves = 32, BoardSize = 64
  };
  using Moves = std::array<int, MaxValidMoves>;
  using Boards = std::array<Board, MaxValidMoves>;
  using Set = std::bitset<BoardSize>;
  static constexpr char BlackCell = '*';
  static constexpr char WhiteCell = 'o';
  static constexpr char EmptyCell = '.';

  // construct a Board with initial '4 disk' position
  Board() : black_((1LL << PosE4) | (1LL << PosD5)), white_((1LL << PosD4) | (1LL << PosE5)) {}

  // construct a Board from a 64 length char representation where:
  //   . = empty cell
  //   * = black cell
  //   o = white cell
  // 'initialEmpty' can be used to allow shorted stings when making boards (helpful for testing), i.e.:
  //   Board(63, "o") creates a board with just a single whith cell in the bottom right corner
  // partial boards are also fine: any cells beyond the end of the string are assumed to be empty, i.e.:
  //   Board(".xo....o") creates a board with some values in the first row, but the rest empty
  explicit Board(const std::string&, int initialEmpty = 0);

  // simple toString function to help with testing (returns a 64 length string)
  std::string toString() const;

  auto blackCount() const { return black_.count(); }
  auto whiteCount() const { return white_.count(); }
  const Set& black() const { return black_; }
  const Set& white() const { return white_; }

  // get list of valid moves for a given color
  std::vector<std::string> validMoves(Color) const;

  // fill 'moves' with positions of valid moves and 'boards' with the corresponding new board for each move
  // and return the total number of valid moves (method used by ComputerPlayer)
  int validMoves(Color, Moves& moves, Boards& boards) const;

  bool hasValidMoves(Color) const;
  bool hasValidMoves() const { return hasValidMoves(Color::Black) || hasValidMoves(Color::White); }
  void printGameResult() const;

  // Set updates Board to reflect the new position (including performing flips)
  // and returns the number of disks that were flipped. If no disks would be
  // flipped by the move then Board is not be updated (since the move is
  // illegal). pos should be a value like 'a1' or 'h8' (column letter followed
  // by row number)
  // Negtive numbers are returned for error conditions:
  //   BadLenght: pos string was not length 2
  //   BadColumn: pos string first character was not a value from 'a' to 'f'
  //   BadRow: pos string second character was not a value from '1' to '8'
  //   BadCell: the cell represented by pos is already occupied
  int set(const std::string& pos, Color);
 private:
  enum PrivateValues { PosD4 = 27, PosE4, PosD5 = 35, PosE5 };
  static auto posToString(int pos) {
    std::string result(1, 'a' + pos % RowSize);
    result.push_back('1' + pos / RowSize);
    return result;
  }

  bool occupied(int pos) const { return black_.test(pos) || white_.test(pos); }
  bool validMove(int pos, const Set& myValues, const Set& opValues) const;
  int set(int pos, Color c) {
    if (c == Color::Black) return set(pos, black_, white_);
    return set(pos, white_, black_);
  }
  int set(int pos, Set& myValues, Set& opValues);

  Set black_;
  Set white_;
};

inline constexpr const char* toString(Board::Color c) {
  return c == Board::Color::Black ? "Black" : "White";
}
inline std::ostream& operator<<(std::ostream& os, const Board::Color& c) {
  return os << toString(c);
}
// output friendly printing including borders with letters and numbers
std::ostream& operator<<(std::ostream&, const Board&);

}  // namespace othello

#endif  // OTHELLO_BOARD_H
