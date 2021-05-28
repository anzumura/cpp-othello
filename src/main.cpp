#include <iostream>

#include "Board.h"

int main() {
  Board b;
  std::cout << "Pretty Print Initial Board:\n" << b;
  std::cout << "Initial Board String: " << b.toString() << std::endl;
  auto moves = b.validMoves(BoardValue::Black);
  std::cout << "Valid Moves: ";
  for (const auto& m : moves)
    std::cout << m << ' ';
  std::cout << std::endl;
  return 0;
}
