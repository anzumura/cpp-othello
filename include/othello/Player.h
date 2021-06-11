#ifndef OTHELLO_PLAYER_H
#define OTHELLO_PLAYER_H

#include <othello/Score.h>

#include <optional>

#include <boost/asio.hpp>

namespace othello {

class Player {
public:
  using Move = std::optional<std::string>;
  static std::unique_ptr<Player> createPlayer(Board::Color, bool computerOnly = false);

  Player(const Player&) = delete;
  virtual ~Player() = default;

  // 'move' returns the string representation of the move that was made or empty string if player
  // wants to end the game. The previous move is passed in to this method so it can be sent to
  // a remote player if needed. Setting tournament to 'true' suppresses printing board each time
  // Note: move is only called if valid moves exist for this player's color
  Move move(Board&, bool tournament, Move prevMove) const;
  // 'printTotalTime' prints the total time taken in seconds to make moves by this player
  // Mote: time is internally measured in nanoseconds, but rounded to nearest microsecond when printing
  void printTotalTime() const;

  const Board::Color color;
  virtual std::string toString() const { return othello::toString(color); }

protected:
  explicit Player(Board::Color c) : color(c), totalTime(0){};
private:
  virtual Move makeMove(Board&, Move prevMove, int& flips) const = 0;
  virtual void printMove(Move move, int flips, bool tournament) const;
  mutable std::chrono::nanoseconds totalTime;
};

class HumanPlayer : public Player {
public:
  explicit HumanPlayer(Board::Color c) : Player(c){};

private:
  Move makeMove(Board&, Move, int&) const override;
  // no need to print move for human player (since the player would have just typed it in)
  void printMove(Move, int, bool) const override {}
  static const char* errorToString(int);
};

class ComputerPlayer : public Player {
public:
  ComputerPlayer(Board::Color c, int search, bool random, std::shared_ptr<Score> score, bool tournament)
      : Player(c), opColor(Board::opColor(c)), _search(search), _random(random), _score(std::move(score)),
        _tournament(tournament){};
  std::string toString() const override;

private:
  using Moves = std::vector<int>;
  enum Values { Min = -Score::Win - 1, Max = Score::Win + 1 };

  // 'makeMove' gets the set of valid moves if search = 0 or calls 'findMoves' when search > 0
  // and makes either the first move in the list or a randomly chosen one if _random is true
  // Note: ComputerPlayer version of 'makeMove' always returns a move (never empty string)
  Move makeMove(Board&, Move, int&) const override;

  // 'findMoves' returns one or more 'best' moves (based on minMax and values returned from '_score')
  std::vector<std::string> findMoves(const Board&) const;

  // 'minMax' is the recursize min-max algorithm with alpha-beta pruning
  int minMax(const Board&, int, Board::Color, int, int, int) const;

  // 'updateMoves' is used by 'findMoves' to work with sets of moves with the same score value
  static void updateMoves(int score, int move, int& best, Moves& moves) {
    if (score > best) {
      best = score;
      moves.clear();
      moves.push_back(move);
    } else if (score == best)
      moves.push_back(move);
  }
  // 'callScore' and 'callMinMax' are helper functions used by 'findMove' and 'minMax'
  int callScore(const Board& board) const {
    ++_totalScoreCalls;
    return _score->score(board, color);
  }
  int callMinMax(const Board& board, int depth, Board::Color turn, int prevMoves, int alpha, int beta) const {
    if (depth) return minMax(board, depth, turn, prevMoves, alpha, beta);
    return callScore(board);
  }

  const Board::Color opColor;
  const int _search;
  const bool _random;
  const std::shared_ptr<Score> _score;
  const bool _tournament; // suppresses printing when true
  mutable long long _totalScoreCalls = 0;
};

class RemotePlayer : public Player {
public:
  explicit RemotePlayer(Board::Color);

private:
  enum Values { Port = 1234 };
  using tcp = boost::asio::ip::tcp;
  Move makeMove(Board&, Move, int&) const override;

  mutable boost::asio::io_service _service;
  mutable tcp::acceptor _acceptor;
  mutable tcp::socket _socket;
};

} // namespace othello

#endif // OTHELLO_PLAYER_H
