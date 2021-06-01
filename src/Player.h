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

  // move returns true once a move has been made or false if player wants to end the game
  // note: move is only called if valid moves exist for this player's color
  bool move(Board&) const;
  // printTotalTime prints the total time taken to make moves so far by this player
  // note: time is internally tracked in nanoseconds, but converted to microseconds when printing
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
  static const char* errorToString(int);
};

class ComputerPlayer : public Player {
 public:
  ComputerPlayer(Board::Color c, int s) : Player(c), search(s) {};
  std::string toString() const override { return Player::toString() + " with search=" + std::to_string(search); }
 private:
  bool makeMove(Board&) const override;
  std::vector<std::string> findMove(const Board&) const;
  const int search;
};

}  // namespace othello

#endif  // OTHELLO_PLAYER_H
