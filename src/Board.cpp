#include <othello/Board.h>

#include <iomanip>

namespace othello {

namespace {

enum BoardValues {
  FirstPos,
  OneColumn,
  SecondRowEnd = 15,
  SeventhRowStart = 48
};

inline auto rowSizeCheck(size_t x) { return x < Board::Rows; };

// must be > 2rd row to flip up
inline auto canFlipUp(size_t x) { return x > SecondRowEnd; };

// must be < 7th row flip down
inline auto canFlipDown(size_t x) { return x < SeventhRowStart; };

// must be > 2rd column to flip left
inline auto canFlipLeft(size_t x) { return x % Board::Rows > OneColumn; };

// must be < 7th column to flip right
inline auto canFlipRight(size_t x) { return x % Board::Rows < Board::RowSub2; };

constexpr auto TopEdge = [](int x) { return x >= FirstPos; };
constexpr auto BottomEdge = [](int x) { return x < Board::Size; };
constexpr auto LeftEdge = [](int x) {
  return x % Board::Rows != Board::RowSub1;
};
constexpr auto RightEdge = [](int x) { return x % Board::Rows != FirstPos; };

constexpr auto UpLeft = [](int x) { return TopEdge(x) && LeftEdge(x); };
constexpr auto DownLeft = [](int x) { return BottomEdge(x) && LeftEdge(x); };
constexpr auto UpRight = [](int x) { return TopEdge(x) && RightEdge(x); };
constexpr auto DownRight = [](int x) { return BottomEdge(x) && RightEdge(x); };

// lambdas used for checking bounds when finding valid moves or performing flips
constexpr auto UpCheck = std::make_pair(-Board::Rows, TopEdge);
constexpr auto DownCheck = std::make_pair(Board::Rows, BottomEdge);

// LeftCheck must also check >= 0 to avoid 'mod -1' so use same lambda for both
// Left and UpLeft cases
constexpr auto LeftCheck = std::make_pair(-OneColumn, UpLeft);
constexpr auto UpLeftCheck = std::make_pair(-Board::RowAdd1, UpLeft);
constexpr auto DownLeftCheck = std::make_pair(Board::RowSub1, DownLeft);
constexpr auto RightCheck = std::make_pair(OneColumn, RightEdge);
constexpr auto UpRightCheck = std::make_pair(-Board::RowSub1, UpRight);
constexpr auto DownRightCheck = std::make_pair(Board::RowAdd1, DownRight);

// for printing to stream
constexpr auto Border = "\
   a b c d e f g h\n\
 +----------------";

} // namespace

Board::Board(const std::string& str, size_t initialEmpty) {
  assert(initialEmpty <= Size);
  assert(initialEmpty + str.size() <= Size);
  for (auto i = initialEmpty; auto c : str) {
    if (c == BlackCell)
      _black.set(i);
    else if (c == WhiteCell)
      _white.set(i);
    ++i;
  }
}

std::string Board::toString() const {
  std::string result(Size, EmptyCell);
  for (size_t i = 0; i < Size; ++i)
    if (_black[i]) {
      assert(!_white[i]);
      result[i] = BlackCell;
    } else if (_white[i])
      result[i] = WhiteCell;
  return result;
}

Board::Moves Board::validMoves(Color c) const {
  Moves result;
  auto& myVals = c == Color::Black ? _black : _white;
  auto& opVals = c == Color::Black ? _white : _black;
  for (size_t i = 0; i < Size; ++i)
    if (!occupied(i) && validMove(i, myVals, opVals))
      result.emplace_back(posToString(i));
  return result;
}

size_t Board::validMoves(Color c, Boards& boards, Positions& positions) const {
  size_t count = 0;
  Board board(*this);
  for (size_t i = 0; i < Size; ++i)
    if (!occupied(i) && board.set(i, c)) {
      assert(count < MaxValidMoves);
      boards[count] = board;
      positions[count++] = i;
      board = *this;
    }
  return count;
}

bool Board::hasValidMoves(Color c) const {
  auto& myVals = c == Color::Black ? _black : _white;
  auto& opVals = c == Color::Black ? _white : _black;
  for (size_t i = 0; i < Size; ++i)
    if (!occupied(i) && validMove(i, myVals, opVals)) return true;
  return false;
}

bool Board::validMove(size_t pos, const Set& myVals, const Set& opVals) const {
  const auto valid = [&](const auto& c) {
    if (int x = static_cast<int>(pos) + c.first; test(opVals, x)) {
      x += c.first;
      do {
        if (test(myVals, x)) return true;
        if (!test(opVals, x)) break;
        x += c.first;
      } while (c.second(x));
    }
    return false;
  };
  const auto flipUp = canFlipUp(pos);
  if (flipUp && valid(UpCheck)) return true;
  const auto flipDown = canFlipDown(pos);
  return (flipDown && valid(DownCheck)) ||
         (canFlipLeft(pos) &&
          (valid(LeftCheck) || (flipUp && valid(UpLeftCheck)) ||
           (flipDown && valid(DownLeftCheck)))) ||
         (canFlipRight(pos) &&
          (valid(RightCheck) || (flipUp && valid(UpRightCheck)) ||
           (flipDown && valid(DownRightCheck))));
}

int Board::set(const std::string& pos, Color c) {
  if (pos.size() != 2) return BadSize;
  const auto col = static_cast<size_t>(pos[0] - 'a');
  if (!rowSizeCheck(col)) return BadColumn;
  const auto row = static_cast<size_t>(pos[1] - '1');
  if (!rowSizeCheck(row)) return BadRow;
  const auto x = static_cast<size_t>(row * Rows + col);
  if (occupied(x)) return BadCell;
  return set(x, c);
}

int Board::set(size_t posIn, Set& myVals, Set& opVals) {
  auto totalFlipped = 0;
  const auto pos = static_cast<int>(posIn);
  // bounds are already checked before calling 'flip' so can use do-while
  const auto flip = [&](const auto& c) {
    if (auto x = pos + c.first; test(opVals, x)) {
      x += c.first;
      do {
        if (test(myVals, x)) { // found 'op-vals' + 'my-val' so flip backwards
          for (x -= c.first; x != pos; x -= c.first) {
            const auto v = static_cast<size_t>(x);
            assert(!myVals[v]);
            assert(opVals[v]);
            ++totalFlipped;
            myVals.set(v);
            opVals.reset(v);
          }
          break;
        } else if (!test(opVals, x))
          break; // found a space in the chain so nothing to flip
        x += c.first;
      } while (c.second(x));
    }
  };
  // check 8 directions for flips
  const auto flipUp = canFlipUp(posIn);
  if (flipUp) flip(UpCheck);
  const auto flipDown = canFlipDown(posIn);
  if (flipDown) flip(DownCheck);
  if (canFlipLeft(posIn)) {
    flip(LeftCheck);
    if (flipUp) flip(UpLeftCheck);
    if (flipDown) flip(DownLeftCheck);
  }
  if (canFlipRight(posIn)) {
    flip(RightCheck);
    if (flipUp) flip(UpRightCheck);
    if (flipDown) flip(DownRightCheck);
  }
  // set 'posIn' cell (passed into this function) if it resulted in flips
  if (totalFlipped > 0) myVals.set(posIn);
  return totalFlipped;
}

Board::GameResults Board::printGameResult(bool tournament) const {
  const auto bc = blackCount();
  const auto wc = whiteCount();
  if (tournament)
    std::cout << std::setw(2) << bc << "," << std::setw(2) << wc;
  else {
    std::cout << '\n' << *this << '\n';
    if (hasValidMoves(Color::Black) || hasValidMoves(Color::White))
      std::cout << "Game Stopped";
    else
      std::cout << "Game Over";
  }
  std::cout << " - ";
  if (bc == wc) {
    std::cout << "draw!\n";
    return GameResults::Draw;
  }
  if (bc > wc) {
    std::cout << Color::Black << " wins!\n";
    return GameResults::Black;
  }
  std::cout << Color::White << " wins!\n";
  return GameResults::White;
}

std::ostream& operator<<(std::ostream& os, const Board& b) {
  static const char black[] = {' ', Board::BlackCell, '\0'};
  static const char white[] = {' ', Board::WhiteCell, '\0'};
  static const char empty[] = {' ', Board::EmptyCell, '\0'};
  static const auto blackScore = std::string("  ") +
                                 toString(Board::Color::Black) + '(' +
                                 Board::BlackCell + "): ";
  static const auto whiteScore = std::string(", ") +
                                 toString(Board::Color::White) + '(' +
                                 Board::WhiteCell + "): ";
  os << Border;
  for (size_t i = 0; i < Board::Size; ++i) {
    if (i % Board::Rows == 0) os << "\n" << i / Board::Rows + 1 << '|';
    if (b.black(i)) {
      assert(!b.white(i));
      os << black;
    } else
      os << (b.white(i) ? white : empty);
  }
  return os << blackScore << b.blackCount() << whiteScore << b.whiteCount()
            << '\n';
}

} // namespace othello
