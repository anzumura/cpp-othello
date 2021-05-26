#ifndef OTHELLO_BOARD_H
#define OTHELLO_BOARD_H

#include <array>
#include <iostream>

class Board {
public:
  enum Values { Empty, Black, White, Flip };
  
private:
  std::array<std::array<int8_t, 8>, 8> cells();
  friend std::ostream& operator<<(std::ostream&, const Board&);
};

std::ostream& operator<<(std::ostream&, const Board&);

#endif //OTHELLO_BOARD_H
