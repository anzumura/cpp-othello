#include "Board.h"

#include <cassert>
#include <utility>

namespace othello {

namespace {

enum BoardValues {
  FirstPos, OneColumn, SecondRowEnd = 15, SeventhRowStart = 48
};

inline auto rowSizeCheck(int x) { return x >= FirstPos && x < Board::RowSize; };

// must be > 2rd row to flip up
inline auto canFlipUp(int x) { return x > SecondRowEnd; };
// must be < 7th row flip down
inline auto canFlipDown(int x) { return x < SeventhRowStart; };
// must be > 2rd column to flip left
inline auto canFlipLeft(int x) { return x % Board::RowSize > OneColumn; };
// must be < 7th column to flip right
inline auto canFlipRight(int x) { return x % Board::RowSize < Board::RowSizeMinusTwo; };

constexpr auto TopEdge = [](int x) { return x >= FirstPos; };
constexpr auto BottomEdge = [](int x) { return x < Board::BoardSize; };
constexpr auto LeftEdge = [](int x) { return x % Board::RowSize != Board::RowSizeMinusOne; };
constexpr auto RightEdge = [](int x) { return x % Board::RowSize != FirstPos; };

constexpr auto UpLeft = [](int x) { return TopEdge(x) && LeftEdge(x); };
constexpr auto DownLeft = [](int x) { return BottomEdge(x) && LeftEdge(x); };
constexpr auto UpRight = [](int x) { return TopEdge(x) && RightEdge(x); };
constexpr auto DownRight = [](int x) { return BottomEdge(x) && RightEdge(x); };

// lambdas used for checking bounds when finding valid moves or performing flips
constexpr auto UpCheck = std::make_pair(-Board::RowSize, TopEdge);
constexpr auto DownCheck = std::make_pair(Board::RowSize, BottomEdge);
// LeftCheck must also check >= 0 to avoid 'mod -1' so use same lambda for both Left and UpLeft cases
constexpr auto LeftCheck = std::make_pair(-OneColumn, UpLeft);
constexpr auto UpLeftCheck = std::make_pair(-Board::RowSizePlusOne, UpLeft);
constexpr auto DownLeftCheck = std::make_pair(Board::RowSizeMinusOne, DownLeft);
constexpr auto RightCheck = std::make_pair(OneColumn, RightEdge);
constexpr auto UpRightCheck = std::make_pair(-Board::RowSizeMinusOne, UpRight);
constexpr auto DownRightCheck = std::make_pair(Board::RowSizePlusOne, DownRight);

// for printing to stream
constexpr auto Border = "\
   a b c d e f g h\n\
 +----------------";

}  // namespace

Board::Board(const std::string& str, int initialEmpty) {
  assert(initialEmpty >= 0 && initialEmpty <= BoardSize);
  assert(initialEmpty + str.length() <= BoardSize);
  int i = initialEmpty;
  for (auto c : str) {
    if (c == 'x')
      black.set(i);
    else if (c == 'o')
      white.set(i);
    ++i;
  }
}

std::string Board::toString() const {
  std::string result(BoardSize, '.');
  for (int i = 0; i < BoardSize; ++i)
    if (black.test(i)) {
      assert(!white.test(i));
      result[i] = 'x';
    } else if (white.test(i))
      result[i] = 'o';
  return result;
}

std::vector<std::string> Board::validMoves(Color c) const {
  std::vector<std::string> result;
  const Set& myValues = c == Color::Black ? black : white;
  const Set& opValues = c == Color::Black ? white : black;
  for (int i = 0; i < BoardSize; ++i)
    if (!occupied(i) && validMove(i, myValues, opValues))
      result.emplace_back(posToString(i));
  return result;
}

int Board::validMoves(Color c, Moves& moves, Boards& boards) const {
  int count = 0;
  Board board(*this);
  for (int i = 0; i < BoardSize; ++i)
    if (!occupied(i) && board.set(i, c)) {
      assert(count < MaxValidMoves);
      moves[count] = i;
      boards[count++] = board;
      board = *this;
    }
  return count;
}

bool Board::hasValidMoves(Color c) const {
  const Set& myValues = c == Color::Black ? black : white;
  const Set& opValues = c == Color::Black ? white : black;
  for (int i = 0; i < BoardSize; ++i)
    if (!occupied(i) && validMove(i, myValues, opValues))
      return true;
  return false;
}

bool Board::validMove(int pos, const Set& myValues, const Set& opValues) const {
  const auto valid = [&](const auto& c) {
    if (int x = pos + c.first; opValues.test(x)) {
      x += c.first;
      do {
        if (myValues.test(x)) return true;
        if (!opValues.test(x)) break;
        x += c.first;
      } while (c.second(x));
    }
    return false;
  };
  const bool flipUp = canFlipUp(pos);
  if (flipUp && valid(UpCheck)) return true;
  const bool flipDown = canFlipDown(pos);
  return (flipDown && valid(DownCheck)) ||
         (canFlipLeft(pos) &&
          (valid(LeftCheck) || (flipUp && valid(UpLeftCheck)) ||
           (flipDown && valid(DownLeftCheck)))) ||
         (canFlipRight(pos) &&
          (valid(RightCheck) || (flipUp && valid(UpRightCheck)) ||
           (flipDown && valid(DownRightCheck))));
}

int Board::set(const std::string& pos, Color c) {
  if (pos.length() != 2) return BadLength;
  const int col = pos[0] - 'a';
  if (!rowSizeCheck(col)) return BadColumn;
  const int row = pos[1] - '1';
  if (!rowSizeCheck(row)) return BadRow;
  const int x = row * RowSize + col;
  if (occupied(x)) return BadCell;
  return set(x, c);
}

int Board::set(int pos, Set& myValues, Set& opValues) {
  int totalFlipped = 0;
  // bounds are already checked before calling 'flip' so can use do-while
  const auto flip = [&](const auto& c) {
    if (int x = pos + c.first; opValues.test(x)) {
      x += c.first;
      do {
        if (myValues.test(x)) {  // found 'op-vals' + 'my-val' so flip backwards
          for (x -= c.first; x != pos; x -= c.first) {
            assert(!myValues.test(x));
            assert(opValues.test(x));
            ++totalFlipped;
            myValues.set(x);
            opValues.reset(x);
          }
          break;
        } else if (!opValues.test(x)) break;  // found a space in the chain so nothing to flip
        x += c.first;
      } while (c.second(x));
    }
  };
  // check 8 directions for flips
  const bool flipUp = canFlipUp(pos);
  if (flipUp) flip(UpCheck);
  const bool flipDown = canFlipDown(pos);
  if (flipDown) flip(DownCheck);
  if (canFlipLeft(pos)) {
    flip(LeftCheck);
    if (flipUp) flip(UpLeftCheck);
    if (flipDown) flip(DownLeftCheck);
  }
  if (canFlipRight(pos)) {
    flip(RightCheck);
    if (flipUp) flip(UpRightCheck);
    if (flipDown) flip(DownRightCheck);
  }
  // set 'pos' cell (passed into this function) if it resulted in flips
  if (totalFlipped > 0) myValues.set(pos);
  return totalFlipped;
}

void Board::printGameResult() const {
  std::cout << std::endl << *this;
  const auto bc = blackCount();
  const auto wc = whiteCount();
  std::cout << "\nGame Over - ";
  if (bc == wc)
    std::cout << "draw!\n";
  else
    std::cout << (bc > wc ? Color::Black : Color::White) << " wins!\n";
}

std::ostream& operator<<(std::ostream& os, const Board& b) {
  os << Border;
  int i = 0;
  do {
    os << "\n" << i / Board::RowSize + 1 << '|';
    do {
      if (b.black.test(i)) {
        assert(!b.white.test(i));
        os << " x";
      } else if (b.white.test(i))
        os << " o";
      else
        os << " .";
    } while (++i % Board::RowSize);
  } while (i < Board::BoardSize);
  return os << " Black(x): " << b.blackCount() << ", White(o): " << b.whiteCount() << '\n';
}

}  // namespace othello
