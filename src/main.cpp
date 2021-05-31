#include "Board.h"
#include "Player.h"

#include <memory>
#include <vector>

using namespace othello;

void playGame() {
  Board board;
  std::vector<std::unique_ptr<Player>> players;
  for (auto c : Board::Colors)
    players.emplace_back(Player::createPlayer(c));
  for (int player = 0, skippedTurns = 0; skippedTurns < 2; player ^= 1) {
    if (board.hasValidMoves(players[player]->color)) {
      if (skippedTurns) {
        std::cout << std::endl << players[player ^ 1]->color << " has no valid moves - skipping turn\n";
        skippedTurns = 0;
      }
      if (!players[player]->move(board)) return;
    } else ++skippedTurns;
  };
  board.printGameResult();
  for (const auto& p : players)
    p->printTotalTime();
}

int main() {
  playGame();
  return 0;
}
