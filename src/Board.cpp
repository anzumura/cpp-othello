#include "Board.h"

#include <cassert>
#include <utility>

namespace othello {

namespace {

enum BoardValues {
  FirstPos, OneColumn, SecondRowEnd = 15, SeventhRowStart = 48
};

inline auto rowSizeCheck(int x) { return x >= FirstPos && x < Board::Rows; };

// must be > 2rd row to flip up
inline auto canFlipUp(int x) { return x > SecondRowEnd; };
// must be < 7th row flip down
inline auto canFlipDown(int x) { return x < SeventhRowStart; };
// must be > 2rd column to flip left
inline auto canFlipLeft(int x) { return x % Board::Rows > OneColumn; };
// must be < 7th column to flip right
inline auto canFlipRight(int x) { return x % Board::Rows < Board::RowsMinusTwo; };

constexpr auto TopEdge = [](int x) { return x >= FirstPos; };
constexpr auto BottomEdge = [](int x) { return x < Board::Size; };
constexpr auto LeftEdge = [](int x) { return x % Board::Rows != Board::RowsMinusOne; };
constexpr auto RightEdge = [](int x) { return x % Board::Rows != FirstPos; };

constexpr auto UpLeft = [](int x) { return TopEdge(x) && LeftEdge(x); };
constexpr auto DownLeft = [](int x) { return BottomEdge(x) && LeftEdge(x); };
constexpr auto UpRight = [](int x) { return TopEdge(x) && RightEdge(x); };
constexpr auto DownRight = [](int x) { return BottomEdge(x) && RightEdge(x); };

// lambdas used for checking bounds when finding valid moves or performing flips
constexpr auto UpCheck = std::make_pair(-Board::Rows, TopEdge);
constexpr auto DownCheck = std::make_pair(Board::Rows, BottomEdge);
// LeftCheck must also check >= 0 to avoid 'mod -1' so use same lambda for both Left and UpLeft cases
constexpr auto LeftCheck = std::make_pair(-OneColumn, UpLeft);
constexpr auto UpLeftCheck = std::make_pair(-Board::RowsPlusOne, UpLeft);
constexpr auto DownLeftCheck = std::make_pair(Board::RowsMinusOne, DownLeft);
constexpr auto RightCheck = std::make_pair(OneColumn, RightEdge);
constexpr auto UpRightCheck = std::make_pair(-Board::RowsMinusOne, UpRight);
constexpr auto DownRightCheck = std::make_pair(Board::RowsPlusOne, DownRight);

// for printing to stream
constexpr auto Border = "\
   a b c d e f g h\n\
 +----------------";

}  // namespace

Board::Board(const std::string& str, int initialEmpty) {
  assert(initialEmpty >= 0 && initialEmpty <= Size);
  assert(initialEmpty + str.length() <= Size);
  int i = initialEmpty;
  for (auto c : str) {
    if (c == BlackCell)
      black_.set(i);
    else if (c == WhiteCell)
      white_.set(i);
    ++i;
  }
}

std::string Board::toString() const {
  std::string result(Size, EmptyCell);
  for (int i = 0; i < Size; ++i)
    if (black_.test(i)) {
      assert(!white_.test(i));
      result[i] = BlackCell;
    } else if (white_.test(i))
      result[i] = WhiteCell;
  return result;
}

std::vector<std::string> Board::validMoves(Color c) const {
  std::vector<std::string> result;
  const Set& myVals = c == Color::Black ? black_ : white_;
  const Set& opVals = c == Color::Black ? white_ : black_;
  for (int i = 0; i < Size; ++i)
    if (!occupied(i) && validMove(i, myVals, opVals))
      result.emplace_back(posToString(i));
  return result;
}

int Board::validMoves(Color c, Moves& moves, Boards& boards) const {
  int count = 0;
  Board board(*this);
  for (int i = 0; i < Size; ++i)
    if (!occupied(i) && board.set(i, c)) {
      assert(count < MaxValidMoves);
      moves[count] = i;
      boards[count++] = board;
      board = *this;
    }
  return count;
}

bool Board::hasValidMoves(Color c) const {
  const Set& myVals = c == Color::Black ? black_ : white_;
  const Set& opVals = c == Color::Black ? white_ : black_;
  for (int i = 0; i < Size; ++i)
    if (!occupied(i) && validMove(i, myVals, opVals))
      return true;
  return false;
}

bool Board::validMove(int pos, const Set& myVals, const Set& opVals) const {
  const auto valid = [&](const auto& c) {
    if (int x = pos + c.first; opVals.test(x)) {
      x += c.first;
      do {
        if (myVals.test(x)) return true;
        if (!opVals.test(x)) break;
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
  const int x = row * Rows + col;
  if (occupied(x)) return BadCell;
  return set(x, c);
}

int Board::set(int pos, Set& myVals, Set& opVals) {
  int totalFlipped = 0;
  // bounds are already checked before calling 'flip' so can use do-while
  const auto flip = [&](const auto& c) {
    if (int x = pos + c.first; opVals.test(x)) {
      x += c.first;
      do {
        if (myVals.test(x)) {  // found 'op-vals' + 'my-val' so flip backwards
          for (x -= c.first; x != pos; x -= c.first) {
            assert(!myVals.test(x));
            assert(opVals.test(x));
            ++totalFlipped;
            myVals.set(x);
            opVals.reset(x);
          }
          break;
        } else if (!opVals.test(x)) break;  // found a space in the chain so nothing to flip
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
  if (totalFlipped > 0) myVals.set(pos);
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
  static char black[] = { ' ', Board::BlackCell, '\0' };
  static char white[] = { ' ', Board::WhiteCell, '\0' };
  static char empty[] = { ' ', Board::EmptyCell, '\0' };
  static auto blackScore = std::string("  ") + toString(Board::Color::Black) + '(' + Board::BlackCell + "): ";
  static auto whiteScore = std::string(", ") + toString(Board::Color::White) + '(' + Board::WhiteCell + "): ";
  os << Border;
  for (int i = 0; i < Board::Size; ++i) {
    if (i % Board::Rows == 0) os << "\n" << i / Board::Rows + 1 << '|';
    if (b.black().test(i)) {
      assert(!b.white().test(i));
      os << black;
    } else os << (b.white().test(i) ? white : empty);
  }
  return os << blackScore << b.blackCount() << whiteScore << b.whiteCount() << '\n';
}

}  // namespace othello
