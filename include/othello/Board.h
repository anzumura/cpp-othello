#pragma once

#include <array>
#include <bitset>
#include <cassert>
#include <iostream>
#include <vector>

namespace othello {

class Board {
public:
  enum class Color { Black, White };
  static constexpr std::array Colors = {Color::Black, Color::White};
  static constexpr Color opColor(Color c) {
    return c == Color::Black ? Color::White : Color::Black;
  }
  enum Values {
    BadSize = -4,
    BadColumn,
    BadRow,
    BadCell, // see 'set' function for details on error conditions
    RowSub2 = 6,
    RowSub1,
    Rows,
    RowAdd1,
    MaxValidMoves = 32,
    SizeSubRows = 56,
    SizeSub1 = 63,
    Size
  };
  using Moves = std::vector<std::string>;
  using Boards = std::array<Board, MaxValidMoves>;
  using Positions = std::array<size_t, MaxValidMoves>;
  using Set = std::bitset<Size>;
  static constexpr char BlackCell = '*';
  static constexpr char WhiteCell = 'o';
  static constexpr char EmptyCell = '.';

  // construct a Board with initial '4 disk' position
  Board()
      : _black((1LL << PosE4) | (1LL << PosD5)),
        _white((1LL << PosD4) | (1LL << PosE5)) {}

  // construct a Board from a char representation where:
  //   . = empty cell
  //   * = black cell
  //   o = white cell
  // 'initialEmpty' can be used to allow shorted stings when making boards
  // (helpful for testing), i.e.:
  //   Board(63, "o") creates board with a white cell in the bottom right corner
  // partial boards are also fine: any cells beyond the end of the string are
  // assumed to be empty, i.e.:
  //   Board(".xo....o") creates board with some values in the first row
  explicit Board(const std::string&, size_t initialEmpty = 0);
  Board(size_t emptyRows, const std::string& stringLayout)
      : Board(stringLayout, emptyRows * Rows) {}

  // operator== is needed for gMock tests
  auto operator==(const Board& rhs) const {
    return _black == rhs._black && _white == rhs._white;
  }

  // simple toString function to help with testing (returns a 64 size string)
  std::string toString() const;

  // methods used by Score function
  auto blackCount() const { return _black.count(); }
  auto whiteCount() const { return _white.count(); }
  auto black() const { return _black; }
  auto white() const { return _white; }

  // get list of valid moves for a given color
  Moves validMoves(Color) const;

  // fill 'positions' with positions of valid moves and 'boards' with the
  // corresponding new board for each move and return the total number of valid
  // moves (method used by ComputerPlayer)
  size_t validMoves(Color, Boards& boards, Positions& positions) const;

  // search algorithms only need to fill 'positions' for first level so provide
  // an simpler overload
  auto validMoves(Color c, Boards& boards) const {
    size_t count = 0;
    Board board(*this);
    for (size_t i = 0; i < Size; ++i)
      if (!occupied(i) && board.set(i, c)) {
        assert(count < MaxValidMoves);
        boards[count++] = board;
        board = *this;
      }
    return count;
  }

  bool hasValidMoves(Color) const;
  auto hasValidMoves() const {
    return hasValidMoves(Color::Black) || hasValidMoves(Color::White);
  }
  enum class GameResults { White, Black, Draw };
  GameResults printGameResult(bool tournament = false) const;
  auto black(size_t i) const { return _black[i]; }
  auto white(size_t i) const { return _white[i]; }

  // Set updates Board to reflect the new position (including performing flips)
  // and returns the number of disks that were flipped. If no disks would be
  // flipped by the move then Board is not be updated (since the move is
  // illegal). pos should be a value like 'a1' or 'h8' (column letter followed
  // by row number)
  // Negative numbers are returned for error conditions:
  //   BadSize: pos string was not size 2
  //   BadColumn: pos string first character was not a value from 'a' to 'f'
  //   BadRow: pos string second character was not a value from '1' to '8'
  //   BadCell: the cell represented by pos is already occupied
  int set(const std::string& pos, Color);

  static auto posToString(size_t pos) {
    std::string result(1, 'a' + pos % Rows);
    result.push_back('1' + static_cast<char>(pos / Rows));
    return result;
  }

  static bool test(const Set& s, int i) { return s[static_cast<size_t>(i)]; }
  template<typename... Ts> static bool test(const Set& s, int i, Ts... args) {
    return test(s, i) || test(s, args...);
  }
private:
  enum PrivateValues { PosD4 = 27, PosE4, PosD5 = 35, PosE5 };
  bool validMove(size_t pos, const Set& myVals, const Set& opVals) const;
  bool occupied(size_t pos) const { return _black[pos] || _white[pos]; }
  int set(size_t pos, Color c) {
    if (c == Color::Black) return set(pos, _black, _white);
    return set(pos, _white, _black);
  }
  int set(size_t pos, Set& myVals, Set& opVals);

  Set _black;
  Set _white;
};

inline constexpr auto* toString(Board::Color c) {
  return c == Board::Color::Black ? "Black" : "White";
}
inline auto& operator<<(std::ostream& os, const Board::Color& c) {
  return os << toString(c);
}

// output friendly printing including borders with letters and numbers
std::ostream& operator<<(std::ostream&, const Board&);

} // namespace othello
