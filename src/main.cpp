#include "Board.h"
#include "Player.h"

using namespace othello;

int main() {
  Board board;
  std::array players = {
    Player::createPlayer(Board::Color::Black),
    Player::createPlayer(Board::Color::White)
  };
  int player = 0;
  int noValidMoves = false;
  do {
    if (board.hasValidMoves(players[player]->color)) {
      if (noValidMoves) {
        std::cout << std::endl << players[player + 1 % 2]->color << " has no valid moves - skipping turn\n";
        noValidMoves = false;
      }
      if (!players[player]->move(board))
        break;
    } else if (noValidMoves)
      break;
    else
      noValidMoves = true;
    player = (player + 1) % 2;
  } while (true);
  if (noValidMoves) {
    std::cout << std::endl << board;
    int blackCount = board.blackCount();
    int whiteCount = board.whiteCount();
    std::cout << "\nGame Over - ";
    if (blackCount == whiteCount)
      std::cout << "draw\n";
    else
      std::cout << (blackCount > whiteCount ? "black" : "white") << " wins!\n";
  }
  return 0;
}
