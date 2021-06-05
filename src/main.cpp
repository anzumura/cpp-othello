#include "Board.h"
#include "Player.h"
#include "Score.h"

#include <memory>
#include <vector>

using namespace othello;

Board playGame(const std::vector<std::unique_ptr<Player>>& players, bool tournament) {
  Board board;
  for (int player = 0, skippedTurns = 0; skippedTurns < 2; player ^= 1) {
    if (board.hasValidMoves(players[player]->color)) {
      if (skippedTurns) {
        if (!tournament)
          std::cout << std::endl << players[player ^ 1]->color << " has no valid moves - skipping turn\n";
        skippedTurns = 0;
      }
      if (!players[player]->move(board, tournament)) break;
    } else
      ++skippedTurns;
  };
  return board;
}

char getChar(Board::Color c, const std::string& msg, const std::string& choices, bool pred(char), char def) {
  std::string line;
  do {
    std::cout << ">>> " << c << " - " << msg << " (" << choices << ") default '" << def << "': ";
    std::getline(std::cin, line);
    if (line.empty()) return def;
  } while (line.length() != 1 || !pred(line[0]));
  return line[0];
}

std::unique_ptr<Player> createPlayer(Board::Color c, bool& tournament) {
  char type = tournament ? 'c'
                         : getChar(
                             c, "player type", "h=human, c=computer, t=computer tournament",
                             [](char x) { return x == 'h' || x == 'c' || x == 't'; }, 't');
  if (type == 'h') return std::make_unique<HumanPlayer>(c);
  if (type == 't') tournament = true;
  char search = getChar(
    c, "search depth", "0=no search, 1-9=moves", [](char x) { return x >= '0' && x <= '9'; }, '1');
  char random = getChar(
    c, "randomized results", "y/n", [](char x) { return x == 'y' || x == 'n'; }, 'y');
  std::unique_ptr<Score> score = nullptr;
  if (search != '0') {
    type = getChar(
      c, "score type", "f=full heuristic, w=weighted cells", [](char x) { return x == 'f' || x == 'w'; }, 'f');
    if (type == 'f')
      score = std::make_unique<FullScore>();
    else
      score = std::make_unique<WeightedScore>();
  }
  return std::make_unique<ComputerPlayer>(c, search - '0', random == 'y', score, tournament);
}

int main() {
  std::vector<std::unique_ptr<Player>> players;
  bool tournament = false;
  for (auto c : Board::Colors)
    players.emplace_back(createPlayer(c, tournament));
  for (int i = 0; i < (tournament ? 10 : 1); ++i)
    playGame(players, tournament).printGameResult(tournament);
  for (const auto& p : players)
    p->printTotalTime();
  return 0;
}
