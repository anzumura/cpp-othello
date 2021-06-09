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
  static void begin();

private:
  static Board playOneGame(const std::vector<std::unique_ptr<Player>>&, bool);
  static char getChar(Board::Color, const std::string&, const std::string&, bool(char), char);
  static std::unique_ptr<Player> createPlayer(Board::Color, bool&, int&);
};

} // namespace othello

#endif // OTHELLO_GAME_H
