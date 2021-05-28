#include "Board.h"

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

// lambdas used for checking bounds when calculating flips
constexpr auto UpCheck = [](int x) { return x >= 0; };
constexpr auto DownCheck = [](int x) { return x < 64; };
// LeftCheck is omitted on purpose (UpLeftCheck is used for this case)
constexpr auto UpLeftCheck = [](int x) { return x >= 0 && x % 8 != 7; };
constexpr auto DownLeftCheck = [](int x) { return x < 64 && x % 8 != 7; };
constexpr auto RightCheck = [](int x) { return x % 8 != 0; };
constexpr auto UpRightCheck = [](int x) { return x >= 0 && x % 8 != 0; };
constexpr auto DownRightCheck = [](int x) { return x < 64 && x % 8 != 0; };

int Board::set(int pos, Set& myValues, Set& opValues) {
  int totalFlipped = 0;
  // bounds are already checked before calling 'flip' so can use do-while
  const auto flip = [&](int inc, bool pred(int)) {
    if (int x = pos + inc; opValues.test(x)) {
      x += inc;
      do {
        if (myValues.test(x)) {  // found 'op-vals' + 'my-val' so flip backwards
          for (x -= inc; x != pos; x -= inc) {
            ++totalFlipped;
            myValues.set(x);
            opValues.reset(x);
          }
          break;
        } else if (!opValues.test(x))
          break;  // found a space in the chain so nothing to flip
        x += inc;
      } while (pred(x));
    }
  };
  // check 8 directions for flips
  const bool canFlipUp = pos > 15;  // must be > 2rd row to flip up
  if (canFlipUp) flip(-8, UpCheck);
  const bool canFlipDown = pos < 48;  // must be < 7th row flip down
  if (canFlipDown) flip(8, DownCheck);
  // must be > 2rd column to flip left
  if (pos % 8 > 1) {
    flip(-1, UpLeftCheck);  // must also check >= 0 so use UpLeftCheck
    if (canFlipUp) flip(-9, UpLeftCheck);
    if (canFlipDown) flip(7, DownLeftCheck);
  }
  // must be < 7th column to flip right
  if (pos % 8 < 6) {
    flip(1, RightCheck);
    if (canFlipUp) flip(-7, UpRightCheck);
    if (canFlipDown) flip(9, DownRightCheck);
  }
  // set 'pos' cell (passed into this function) if it resulted in flips
  if (totalFlipped > 0) myValues.set(pos);
  return totalFlipped;
}

// for printing to stream
constexpr auto Border = "\
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
