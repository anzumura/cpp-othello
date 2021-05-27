#include "Board.h"

Board::Board(const char* str) {
  const char* p = str;
  for(int i = 0; i < 64; ++i, ++p)
    if(*p == 'x')
      black.set(i);
    else if(*p == 'o')
      white.set(i);
}

std::string Board::toString() const {
  std::string result(64, '.');
  for(int i = 0; i < 64; ++i)
    if(black.test(i)) {
      assert(!white.test(i));
      result[i] = 'x';
    } else if(white.test(i))
      result[i] = 'o';
  return result;
}

int Board::set(const char* pos, BoardValue value) {
  if(strlen(pos) != 2)
    return 0;
  const int col = pos[0] - 'a';
  if(col < 0 || col > 7)
    return 0;
  const int row = pos[1] - '1';
  if(row < 0 || row > 7)
    return 0;
  const int z = row * 8 + col;
  if(black.test(z) || white.test(z))
    return 0;
  if(value == BoardValue::Black)
    return set(z, black, white);
  return set(z, white, black);
}

int Board::set(int pos, Set& myValues, Set& opValues) {
  int totalFlipped = 0;
  const auto flip = [&](int inc, bool pred(int)) {
    int x = pos + inc;
    if(opValues.test(x)) {
      for(x += inc; pred(x); x += inc) {
        if(myValues.test(x)) {
          // found chain of 'op-values' followed by 'my-value' so work backwards and flip
          for(x -= inc; x != pos; x -= inc) {
            ++totalFlipped;
            myValues.set(x);
            opValues.reset(x);
          }
          break;
        } else if(!opValues.test(x))
          break; // found a space in the chain so nothing to flip
      }
    }
  };
  // check 8 directions for flips
  const bool canFlipUp = pos > 15; // need to be at least in the 3rd row
  if(canFlipUp)
    flip(-8, [](int x){ return x >= 0; }); // up
  const bool canFlipDown = pos < 48; // need to be in less than the 7th row
  if(canFlipDown)
    flip(8, [](int x){ return x < 64; }); // down
  if(pos % 8 > 1) {
    flip(-1, [](int x){ return x >= 0 && x % 8 != 7; }); // left
    if(canFlipUp)
      flip(-9, [](int x){ return x >= 0 && x % 8 != 7; }); // up-left
    if(canFlipDown)
      flip(7, [](int x){ return x % 8 != 7; }); // down-left
  }
  if(pos % 8 < 6) {
    flip(1, [](int x){ return x % 8 != 0; }); // right
    if(canFlipUp)
      flip(-7, [](int x){ return x >= 0 && x % 8 != 0; }); // up-right
    if(canFlipDown)
      flip(9, [](int x){ return x % 8 != 0; }); // down-right
  }
  // set the value of cell if there were any flips
  if(totalFlipped > 0)
    myValues.set(pos);
  return totalFlipped;
}

// for printing to stream
constexpr auto Border = "\
+-+-----------------+-+\n\
| | A B C D E F G H | |\n\
+-+-----------------+-+\n";

std::ostream& operator<<(std::ostream& os, const Board& b) {
  os << Border;
  for(int i = 0; i < 8; ++i) {
    os << '|' << i + 1 << "| ";
    for(int j = 0; j < 8; ++j)
      if(b.black.test(i * 8 + j)) {
        assert(!b.white.test(i * 8 + j));
        os << "x ";
      } else if(b.white.test(i * 8 + j))
        os << "o ";
      else
        os << ". ";
    os << "|" << i + 1 << "|\n";
  }
  return os << Border;
}
