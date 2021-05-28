#include <utility>

#include "Board.h"

// LeftCheck also must check >= 0 so use this lambda for both cases
constexpr auto UpLeftLambda = [](int x) { return x >= 0 && x % 8 != 7; };

// lambdas used for checking bounds when calculating flips
constexpr auto UpCheck = std::make_pair(-8, [](int x) { return x >= 0; });
constexpr auto DownCheck = std::make_pair(8, [](int x) { return x < 64; });
constexpr auto LeftCheck = std::make_pair(-1, UpLeftLambda);
constexpr auto UpLeftCheck = std::make_pair(-9, UpLeftLambda);
constexpr auto DownLeftCheck = std::make_pair(7, [](int x) { return x < 64 && x % 8 != 7; });
constexpr auto RightCheck = std::make_pair(1, [](int x) { return x % 8 != 0; });
constexpr auto UpRightCheck = std::make_pair(-7, [](int x) { return x >= 0 && x % 8 != 0; });
constexpr auto DownRightCheck = std::make_pair(9, [](int x) { return x < 64 && x % 8 != 0; });

Board::Board(const char* str) {
  const char* p = str;
  for (int i = 0; i < 64; ++i, ++p)
    if (*p == 'x')
      black.set(i);
    else if (*p == 'o')
      white.set(i);
}

std::string Board::toString() const {
  std::string result(64, '.');
  for (int i = 0; i < 64; ++i)
    if (black.test(i)) {
      assert(!white.test(i));
      result[i] = 'x';
    } else if (white.test(i))
      result[i] = 'o';
  return result;
}

std::vector<std::string> Board::validMoves(BoardValue value) const {
  std::vector<std::string> result;
  const Set& myValues = value == BoardValue::Black ? black : white;
  const Set& opValues = value == BoardValue::Black ? white : black;
  for (int i = 0; i < 64; ++i)
    if (!black.test(i) && !white.test(i) && validMove(i, myValues, opValues))
      result.emplace_back(posToString(i));
  return result;
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
  const bool canFlipUp = pos > 15;  // must be > 2rd row to flip up
  if (canFlipUp && valid(UpCheck))
    return true;
  const bool canFlipDown = pos < 48;  // must be < 7th row flip down
  return (canFlipDown && valid(DownCheck)) ||
         (pos % 8 > 1 &&  // must be > 2rd column to flip left
          (valid(LeftCheck) || (canFlipUp && valid(UpLeftCheck)) ||
           (canFlipDown && valid(DownLeftCheck)))) ||
         (pos % 8 < 6 &&  // must be < 7th column to flip right
          (valid(RightCheck) || (canFlipUp && valid(UpRightCheck)) ||
           (canFlipDown && valid(DownRightCheck))));
}

int Board::set(const char* pos, BoardValue value) {
  if (strlen(pos) != 2) return 0;
  const int col = pos[0] - 'a';
  if (col < 0 || col > 7) return 0;
  const int row = pos[1] - '1';
  if (row < 0 || row > 7) return 0;
  const int z = row * 8 + col;
  if (black.test(z) || white.test(z)) return 0;
  if (value == BoardValue::Black) return set(z, black, white);
  return set(z, white, black);
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
            ++totalFlipped;
            myValues.set(x);
            opValues.reset(x);
          }
          break;
        } else if (!opValues.test(x))
          break;  // found a space in the chain so nothing to flip
        x += c.first;
      } while (c.second(x));
    }
  };
  // check 8 directions for flips
  const bool canFlipUp = pos > 15;  // must be > 2rd row to flip up
  if (canFlipUp) flip(UpCheck);
  const bool canFlipDown = pos < 48;  // must be < 7th row flip down
  if (canFlipDown) flip(DownCheck);
  // must be > 2rd column to flip left
  if (pos % 8 > 1) {
    flip(LeftCheck);
    if (canFlipUp) flip(UpLeftCheck);
    if (canFlipDown) flip(DownLeftCheck);
  }
  // must be < 7th column to flip right
  if (pos % 8 < 6) {
    flip(RightCheck);
    if (canFlipUp) flip(UpRightCheck);
    if (canFlipDown) flip(DownRightCheck);
  }
  // set 'pos' cell (passed into this function) if it resulted in flips
  if (totalFlipped > 0) myValues.set(pos);
  return totalFlipped;
}

// for printing to stream
constexpr auto Border =
    "\
+-+-----------------+-+\n\
| | A B C D E F G H | |\n\
+-+-----------------+-+\n";

std::ostream& operator<<(std::ostream& os, const Board& b) {
  os << Border;
  for (int i = 0; i < 8; ++i) {
    os << '|' << i + 1 << "| ";
    for (int j = 0; j < 8; ++j)
      if (b.black.test(i * 8 + j)) {
        assert(!b.white.test(i * 8 + j));
        os << "x ";
      } else if (b.white.test(i * 8 + j))
        os << "o ";
      else
        os << ". ";
    os << "|" << i + 1 << "|\n";
  }
  return os << Border;
}
