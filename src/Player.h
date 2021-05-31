#ifndef OTHELLO_PLAYER_H
#define OTHELLO_PLAYER_H

#include <chrono>
#include <memory>
#include <string>

#include "Board.h"

namespace othello {

class Player {
 public:
  static std::unique_ptr<Player> createPlayer(Board::Color);

  Player(const Player&) = delete;
  virtual ~Player() = default;

  bool move(Board&) const;
  void printTotalTime() const;

  const Board::Color color;
  virtual std::string toString() const { return othello::toString(color); }
 protected:
  explicit Player(Board::Color c) : color(c), totalTime(0) {};
 private:
  virtual bool makeMove(Board&) const = 0;
  static char getChar(const std::string&, bool(char));
  mutable std::chrono::nanoseconds totalTime;
};

class HumanPlayer : public Player {
 public:
  explicit HumanPlayer(Board::Color c) : Player(c) {};
 private:
  bool makeMove(Board&) const override;
};

class ComputerPlayer : public Player {
 public:
  ComputerPlayer(Board::Color c, int s) : Player(c), search(s) {};
  std::string toString() const override { return Player::toString() + " with search=" + std::to_string(search); }
 private:
  bool makeMove(Board&) const override;
  const int search;
};

}  // namespace othello

#endif  // OTHELLO_PLAYER_H
