#include <iostream>

#include "Board.h"

int main() {
  Board b;
  std::cout << "Pretty Print Initial Board:\n" << b;
  std::cout << "Initial Board String: " << b.toString() << std::endl;
  return 0;
}
