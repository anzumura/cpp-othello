#pragma once

#include <othello/Score.h>

#include <optional>

#include <boost/asio.hpp>

namespace othello {

class Player {
public:
  using Move = std::optional<std::string>;
  static std::unique_ptr<Player> createPlayer(Board::Color,
                                              bool computerOnly = false);

  Player(const Player&) = delete;
  virtual ~Player() = default;

  // 'move' returns the string representation of the move that was made or empty
  // string if player wants to end the game. prevMoves is passed in to this
  // method so it can be sent to a remote player if needed. Setting tournament
  // to 'true' suppresses printing board each time. Note: move is only called if
  // valid moves exist for this player's color Note: prevMoves will be empty if
  // the other player had no valid moves and can contain more than one entry if
  // the other player played multiple times in a row.
  Move move(Board&, bool tournament, const Board::Moves& prevMoves) const;

  // 'gameOver' is called when the game is finished and the final board position
  // and any moves made by the other player are passed in (for communication to
  // RemotePlayer for example)
  virtual void gameOver(const Board&,
                        const Board::Moves& = Board::Moves()) const {}

  // 'printTotalTime' prints the total time taken in seconds to make moves by
  // this player Mote: time is internally measured in nanoseconds, but rounded
  // to nearest microsecond when printing
  void printTotalTime() const;

  const Board::Color color;
  virtual std::string toString() const { return othello::toString(color); }
protected:
  explicit Player(Board::Color c) : color(c), totalTime(0){};
  static const char* errorToString(int);
private:
  virtual Move makeMove(Board&, const Board::Moves& prevMoves,
                        int& flips) const = 0;
  virtual void printMove(Move move, int flips, bool tournament) const;
  mutable std::chrono::nanoseconds totalTime;
};

class HumanPlayer : public Player {
public:
  explicit HumanPlayer(Board::Color c) : Player(c){};
private:
  Move makeMove(Board&, const Board::Moves&, int&) const override;

  // no need to print for human player (since player would have just typed it)
  void printMove(Move, int, bool) const override {}
};

class ComputerPlayer : public Player {
public:
  ComputerPlayer(Board::Color c, int search, bool random,
                 std::shared_ptr<Score> score)
      : Player(c), opColor(Board::opColor(c)), _search(search), _random(random),
        _score(std::move(score)){};
  std::string toString() const override;
private:
  using Moves = std::vector<int>;
  enum Values { Min = -Score::Win - 1, Max = Score::Win + 1 };

  // 'makeMove' gets the set of valid moves if search = 0 or calls 'findMoves'
  // when search > 0 and makes either the first move in the list or a randomly
  // chosen one if _random is true Note: ComputerPlayer version of 'makeMove'
  // always returns a move (never empty string)
  Move makeMove(Board&, const Board::Moves&, int&) const override;

  // 'findMoves' returns one or more 'best' moves (based on minMax and values
  // returned from '_score')
  Board::Moves findMoves(const Board&) const;

  // 'minMax' is the recursize min-max algorithm with alpha-beta pruning
  int minMax(const Board&, int, Board::Color, int, int, int) const;

  // 'updateMoves' is used by 'findMoves' to work with sets of moves with the
  // same score value
  static void updateMoves(int score, int move, int& best, Moves& moves) {
    if (score > best) {
      best = score;
      moves.clear();
      moves.push_back(move);
    } else if (score == best)
      moves.push_back(move);
  }

  // 'callScore' and 'callMinMax' are used by 'findMove' and 'minMax'
  auto callScore(const Board& board) const {
    ++_totalScoreCalls;
    return _score->score(board, color);
  }
  auto callMinMax(const Board& board, int depth, Board::Color turn,
                  int prevMoves, int alpha, int beta) const {
    if (depth) return minMax(board, depth, turn, prevMoves, alpha, beta);
    return callScore(board);
  }

  const Board::Color opColor;
  const int _search;
  const bool _random;
  const std::shared_ptr<Score> _score;
  mutable long long _totalScoreCalls = 0;
};

class RemotePlayer : public Player {
public:
  explicit RemotePlayer(Board::Color, bool debug = false);
  void gameOver(const Board&, const Board::Moves&) const override;
private:
  enum Values { Port = 1234 };
  using tcp = boost::asio::ip::tcp;

  Move makeMove(Board&, const Board::Moves&, int&) const override;

  void waitForConnection(const Board&, const Board::Moves&) const;

  // helper functions for sending and receiving data
  std::string get() const;
  void send(const std::string&) const;
  void send(const Board& b) const {
    if (_print) send(b.toString());
  }
  void send(const Board::Moves& moves) const {
    std::string out;
    for (auto& s : moves) out += s;
    send(out);
  }
  void opFailed(const char* msg) const;

  const bool _debug;
  mutable bool _print = false;
  mutable boost::asio::io_service _service;
  mutable tcp::acceptor _acceptor;
  mutable tcp::socket _socket;
  mutable boost::asio::streambuf _inputBuffer;
  mutable std::istream _input;
  mutable boost::system::error_code _error;
};

} // namespace othello
