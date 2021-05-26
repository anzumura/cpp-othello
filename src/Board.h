#ifndef OTHELLO_BOARD_H
#define OTHELLO_BOARD_H

#include <array>
#include <iostream>

enum class BoardValue:int8_t { Empty, Black, White };

class Board {
public:
  // construct a Board with initial '4 disk' position
  Board();
  // construct a Board from a 64 length char representation where:
  //   . = empty cell
  //   x = black cell
  //   o = white cell
  explicit Board(const char*);
  // simple toString function to help with testing (returns a 64 length string)
  std::string toString() const;

  void set(int x, int y, BoardValue v) {
    cells[x][y] = v;
  }
private:
  std::array<std::array<BoardValue, 8>, 8> cells;
  friend std::ostream& operator<<(std::ostream&, const Board&);
};

// output friendly printing including borders with letters and numbers
std::ostream& operator<<(std::ostream&, const Board&);

#endif //OTHELLO_BOARD_H
