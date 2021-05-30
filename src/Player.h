#ifndef OTHELLO_PLAYER_H
#define OTHELLO_PLAYER_H

#include <memory>

#include "Board.h"

namespace othello {

class Player {
 public:
  static std::unique_ptr<Player> createPlayer(Board::Color);

  Player(const Player&) = delete;
  virtual ~Player() = default;

  bool move(Board&) const;

  const Board::Color color;
 protected:
  explicit Player(Board::Color c) : color(c) {};
 private:
  virtual bool makeMove(Board&) const = 0;
};

class HumanPlayer : public Player {
 public:
  explicit HumanPlayer(Board::Color c) : Player(c) {};
 private:
  bool makeMove(Board&) const override;
};

class ComputerPlayer : public Player {
 public:
  explicit ComputerPlayer(Board::Color c) : Player(c) {};
 private:
  bool makeMove(Board&) const override;
};

}  // namespace othello

#endif  // OTHELLO_PLAYER_H
