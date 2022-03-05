#pragma once

#include <othello/Player.h>

namespace othello {

class Game {
public:
  Game() : _matches(0), _hasRemotePlayer(false) {}

  // 'begin' prompts for number of matches (for a 'tournament') and player
  // types, then starts the game(s) matches is 0 for a non-tournament style
  // interactive game (showing the board each turn)
  void begin();
private:
  static char getChar(Board::Color, const std::string&, const std::string&,
                      bool(char), char);
  Board playOneGame();

  // createPlayer also updates _matches and _tournament depending on user input
  std::unique_ptr<Player> createPlayer(Board::Color);

  size_t _matches;
  bool _hasRemotePlayer;
  std::vector<std::unique_ptr<Player>> _players;
};

} // namespace othello
