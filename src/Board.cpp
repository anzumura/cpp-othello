#include "Board.h"

constexpr auto E(BoardValue::Empty);
constexpr auto B(BoardValue::Black);
constexpr auto W(BoardValue::White);
constexpr std::array<BoardValue, 8> RowE = {E, E, E, E, E, E, E, E};
constexpr std::array<BoardValue, 8> Row4 = {E, E, E, W, B, E, E, E};
constexpr std::array<BoardValue, 8> Row5 = {E, E, E, B, W, E, E, E};

Board::Board() : cells{RowE, RowE, RowE, Row4, Row5, RowE, RowE, RowE} {};

Board::Board(const char* str) {
  const char* p = str;
  for(int i = 0; i < 8; ++i)
    for(int j = 0; j < 8; ++j)
      set(i, j, *p == '.' ? BoardValue::Empty : *p == 'x' ? BoardValue::Black : BoardValue::White);
}

std::string Board::toString() const {
  std::string result(64, '.');
  int count = 0;
  for(auto i : cells)
    for(auto j : i) {
      if(j == BoardValue::Black)
        result[count] = 'x';
      else if(j == BoardValue::White)
        result[count] = 'o';
      ++count;
    }
  return result;
}

// for printing to stream
constexpr auto Border = "\
+-+-----------------+-+\n\
| | A B C D E F G H | |\n\
+-+-----------------+-+\n";

std::ostream& operator<<(std::ostream& os, const Board& b) {
  os << Border;
  int row = 1;
  for(const auto& i: b.cells) {
    os << '|' << row << "| ";
    for(auto j : i) {
      switch(j) {
        case BoardValue::Empty: os << ". "; break;
        case BoardValue::Black: os << "x "; break;
        case BoardValue::White: os << "o "; break;
      }
    }
    os << "|" << row << "|\n";
    ++row;
  }
  return os << Border;
}
