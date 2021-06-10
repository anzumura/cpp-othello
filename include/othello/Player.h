#ifndef OTHELLO_PLAYER_H
#define OTHELLO_PLAYER_H

#include <othello/Score.h>

namespace othello {

class Player {
public:
  static std::unique_ptr<Player> createPlayer(Board::Color, bool computerOnly = false);

  Player(const Player&) = delete;
  virtual ~Player() = default;

  // 'move' returns true once a move has been made or false if player wants to end the game
  // setting tournament to 'true' suppresses printing board each time
  // note: move is only called if valid moves exist for this player's color
  bool move(Board&, bool tournament) const;
  // 'printTotalTime' prints the total time taken in seconds to make moves by this player
  // note: time is internally measured in nanoseconds, but rounded to nearest microsecond when printing
  void printTotalTime() const;

  const Board::Color color;
  virtual std::string toString() const { return othello::toString(color); }

protected:
  explicit Player(Board::Color c) : color(c), totalTime(0){};

private:
  virtual bool makeMove(Board&) const = 0;
  mutable std::chrono::nanoseconds totalTime;
};

class HumanPlayer : public Player {
public:
  explicit HumanPlayer(Board::Color c) : Player(c){};

private:
  bool makeMove(Board&) const override;
  static const char* errorToString(int);
};

class ComputerPlayer : public Player {
public:
  ComputerPlayer(Board::Color c, int search, bool random, std::shared_ptr<Score> score, bool tournament)
      : Player(c), opColor(Board::opColor(c)), _search(search), _random(random), _score(std::move(score)),
        _tournament(tournament){};
  std::string toString() const override;

private:
  bool makeMove(Board&) const override;
  std::vector<std::string> findMove(const Board&) const;
  int callMinMax(const Board& board, int depth, Board::Color turn, int prevMoves, int alpha, int beta) const {
    if (depth) return minMax(board, depth, turn, prevMoves, alpha, beta);
    ++_totalScoreCalls;
    return _score->score(board, color);
  }
  int minMax(const Board&, int, Board::Color, int, int, int) const;

  const Board::Color opColor;
  const int _search;
  const bool _random;
  const std::shared_ptr<Score> _score;
  const bool _tournament; // suppresses printing when true
  mutable long long _totalScoreCalls = 0;
};

} // namespace othello

#endif // OTHELLO_PLAYER_H
