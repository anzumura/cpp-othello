#ifndef OTHELLO_GAME_H
#define OTHELLO_GAME_H

#include <othello/Board.h>
#include <othello/Player.h>

#include <memory>
#include <string>
#include <vector>

namespace othello {

class Game {
public:
  Game() : _tournament(false), _matches(1) {}
  // begin prompts for number of matches and player types and then start the game(s)
  void begin();

private:
  static char getChar(Board::Color, const std::string&, const std::string&, bool(char), char);
  Board playOneGame();
  std::unique_ptr<Player> createPlayer(Board::Color);

  bool _tournament;
  int _matches;
  std::vector<std::unique_ptr<Player>> _players;
};

} // namespace othello

#endif // OTHELLO_GAME_H
