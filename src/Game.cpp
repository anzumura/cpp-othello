#include <othello/Game.h>

#include <iomanip>

namespace othello {

void Game::begin() {
  int gameCount = 1, blackWins = 0, whiteWins = 0, draws = 0, blackPieces = 0, whitePieces = 0;
  for (auto c : Board::Colors)
    _players.emplace_back(createPlayer(c));
  do {
    if (_matches) {
      const int width = _matches < 10 ? 2 : _matches < 100 ? 3 : _matches < 1000 ? 4 : 5;
      std::cout << "Starting Game: " << std::setw(width) << std::left << gameCount << "... ";
      std::cout.flush();
    }
    Board board = playOneGame();
    switch (board.printGameResult(_matches)) {
    case Board::GameResults::Black:
      ++blackWins;
      break;
    case Board::GameResults::White:
      ++whiteWins;
      break;
    case Board::GameResults::Draw:
      ++draws;
      break;
    }
    blackPieces += board.blackCount();
    whitePieces += board.whiteCount();
  } while (++gameCount <= _matches);
  if (_matches > 1)
    std::cout << ">>> Black Wins: " << blackWins << ", White Wins: " << whiteWins << ", Draws: " << draws
              << "\n>>> Black Pieces: " << blackPieces << ", White Pieces: " << whitePieces << '\n';
  for (const auto& p : _players)
    p->printTotalTime();
}

Board Game::playOneGame() {
  Board board;
  for (int player = 0, skippedTurns = 0; skippedTurns < 2; player ^= 1) {
    if (board.hasValidMoves(_players[player]->color)) {
      if (skippedTurns) {
        if (!_matches) std::cout << std::endl << _players[player ^ 1]->color << " has no valid moves - skipping turn\n";
        skippedTurns = 0;
      }
      if (!_players[player]->move(board, _matches)) break;
    } else
      ++skippedTurns;
  };
  return board;
}

char Game::getChar(Board::Color c, const std::string& msg, const std::string& choices, bool pred(char), char def) {
  std::string line;
  do {
    std::cout << ">>> " << c << " - " << msg << " (" << choices << ") default '" << def << "': ";
    std::getline(std::cin, line);
    if (line.empty()) return def;
  } while (line.length() != 1 || !pred(line[0]));
  return line[0];
}

std::unique_ptr<Player> Game::createPlayer(Board::Color c) {
  char type = _matches ? 'c'
                       : getChar(
                           c, "player type", "h=human, c=computer or tournaments: w=1, x=10 games, y=100, z=1000",
                           [](char x) { return x == 'h' || x == 'c' || (x >= 'w' && x <= 'z'); }, 'y');
  if (type == 'h') return std::make_unique<HumanPlayer>(c);
  if (type == 'w')
    _matches = 1;
  else if (type == 'x')
    _matches = 10;
  else if (type == 'y')
    _matches = 100;
  else if (type == 'z')
    _matches = 1000;
  char search = getChar(
    c, "search depth", "0=no search, 1-9=moves", [](char x) { return x >= '0' && x <= '9'; }, '1');
  char random = getChar(
    c, "randomized results", "y/n", [](char x) { return x == 'y' || x == 'n'; }, 'y');
  std::shared_ptr<Score> score = nullptr;
  if (search != '0') {
    type = getChar(
      c, "score type", "f=full heuristic, w=weighted cells", [](char x) { return x == 'f' || x == 'w'; }, 'f');
    if (type == 'f')
      score = std::make_shared<FullScore>();
    else
      score = std::make_shared<WeightedScore>();
  }
  return std::make_unique<ComputerPlayer>(c, search - '0', random == 'y', score, _matches);
}

} // namespace othello