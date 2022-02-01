#include <othello/Player.h>
#include <othello/Score.h>

#include <iomanip>
#include <random>
#include <sstream>

namespace othello {

Player::Move Player::move(Board& board, bool tournament, const Board::Moves& prevMoves) const {
  assert(board.hasValidMoves(color));
  if (!tournament) std::cout << '\n' << board << '\n';
  auto flips = 0;
  const auto start = std::chrono::high_resolution_clock::now();
  const auto move = makeMove(board, prevMoves, flips);
  totalTime += std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
  printMove(move, flips, tournament);
  return move;
}

void Player::printMove(Move move, int flips, bool tournament) const {
  if (move) {
    assert(flips > 0);
    if (!tournament)
      std::cout << color << " played at: " << *move << " (" << flips << " flip" << (flips > 1 ? "s" : "") << ")\n";
  }
}

void Player::printTotalTime() const {
  std::cout << "Total time for " << toString() << ": " << std::fixed << std::setprecision(6)
            << totalTime.count() / 1'000'000'000.0 << " seconds\n";
}

const char* Player::errorToString(int flips) {
  switch (flips) {
  case Board::BadLength: return "location must be 2 characters";
  case Board::BadColumn: return "column must be a value from 'a' to 'h'";
  case Board::BadRow: return "row must be a value from '1' to '8'";
  case Board::BadCell: return "cell already occupied";
  }
  return "must flip at least one piece";
}

Player::Move HumanPlayer::makeMove(Board& board, const Board::Moves&, int& flips) const {
  do {
    std::cout << "enter " << color << "'s move: ";
    std::string line;
    std::getline(std::cin, line);
    if (line.length() == 1) {
      if (line[0] == 'q') return {};
      if (line[0] == 'v') {
        const auto moves = board.validMoves(color);
        std::cout << "  valid moves are:";
        for (auto& m : moves) std::cout << " " << m;
        std::cout << '\n';
      }
    } else {
      flips = board.set(line, color);
      if (flips > 0) return line;
      std::cout << "  invalid move: '" << line << "' - " << errorToString(flips)
                << "\n  please enter a location (eg 'a1' or 'h8'), 'q' to quit or 'v' to print valid moves\n";
    }
  } while (true);
}

std::string ComputerPlayer::toString() const {
  std::stringstream ss;
  ss << Player::toString();
  if (_score) ss << " (" << _score->toString() << ")";
  ss << " with";
  if (_random) ss << " randomized";
#ifdef __clang__
  // g++ uses libstdc++ (instead of libc++) which only to understands locale "C". Any other value causes
  // a runtime exception: locale::facet::_S_create_c_locale
  std::locale loc("en_US.UTF-8");
  ss.imbue(loc);
#endif
  ss << " search=" << _search << " (score called " << _totalScoreCalls << ")";
  return ss.str();
}

Player::Move ComputerPlayer::makeMove(Board& board, const Board::Moves&, int& flips) const {
  static std::random_device rd;
  static std::mt19937 gen(rd());

  const auto moves = _search == 0 ? board.validMoves(color) : findMoves(board);
  assert(!moves.empty());
  auto move = 0;
  if (_random && moves.size() > 1) {
    std::uniform_int_distribution<> dis(0, moves.size() - 1);
    move = dis(gen);
  }
  flips = board.set(moves[move], color);
  return moves[move];
}

Board::Moves ComputerPlayer::findMoves(const Board& board) const {
  Board::Boards boards;
  Board::Positions positions;
  const auto moves = board.validMoves(color, boards, positions);
  const auto nextLevel = _search - 1;
  // return more than one position if moves have the same score
  Moves bestMoves;
  for (int i = 0, best = Min; i < moves; ++i)
    updateMoves(callMinMax(boards[i], nextLevel, opColor, moves, best, Max), i, best, bestMoves);
  // if there are multiple moves with the same score then only return ones with the best 'first move' score
  if (bestMoves.size() > 1) {
    int best = Min;
    std::vector<int> newBestMoves;
    for (auto i : bestMoves) updateMoves(callScore(boards[i]), i, best, newBestMoves);
    bestMoves = newBestMoves;
  }
  Board::Moves results;
  for (auto i : bestMoves) results.emplace_back(Board::posToString(positions[i]));
  return results;
}

int ComputerPlayer::minMax(const Board& board, int depth, Board::Color turn, int prevMoves, int alpha, int beta) const {
  Board::Boards boards;
  const auto moves = board.validMoves(turn, boards);
  const auto nextLevel = depth - 1;
  // if no valid moves for current player then go to next level unless there were no valid moves
  // for previous level - in this case stop traversing and return score (by setting depth to 0)
  if (moves == 0) return callMinMax(board, prevMoves ? nextLevel : 0, Board::opColor(turn), 0, alpha, beta);
  // maximizing player
  if (turn == color) {
    int best = Min;
    for (auto i = 0; i < moves && best < beta; ++i, alpha = std::max(alpha, best))
      best = std::max(best, callMinMax(boards[i], nextLevel, opColor, moves, alpha, beta));
    return best;
  }
  // minimizing player
  int best = Max;
  for (auto i = 0; i < moves && best > alpha; ++i, beta = std::min(beta, best))
    best = std::min(best, callMinMax(boards[i], nextLevel, color, moves, alpha, beta));
  return best;
}

RemotePlayer::RemotePlayer(Board::Color c, bool debug)
  : Player(c), _debug(debug), _acceptor(_service, tcp::endpoint(tcp::v4(), Port)), _socket(_service),
    _input(&_inputBuffer) {}

using namespace boost::asio;

void RemotePlayer::waitForConnection(const Board& board, const Board::Moves& prevMoves) const {
  std::cout << color << ": waiting for initial connection on port " << Port << " ... ";
  std::cout.flush();
  _acceptor.accept(_socket);
  std::cout << "connected\n\n";
  if (auto clientType = get(); clientType == "printBoards") {
    _print = true;
    send(prevMoves);
    // always send initial board string even if previous move is empty, i.e.: we are first player
    send(board.toString());
  } else
    send(prevMoves);
}

void RemotePlayer::gameOver(const Board& board, const Board::Moves& prevMoves) const {
  send("end");
  send(prevMoves);
  if (!prevMoves.empty()) send(board);
}

Player::Move RemotePlayer::makeMove(Board& board, const Board::Moves& prevMoves, int& flips) const {
  if (!_socket.is_open())
    waitForConnection(board, prevMoves);
  else {
    send(prevMoves);
    // only send board string if other player made a move
    if (!prevMoves.empty()) send(board);
  }
  do {
    if (std::string line = get(); line == "q")
      return {};
    else if (line == "v")
      send(board.validMoves(color));
    else {
      flips = board.set(line, color);
      if (flips > 0) {
        send(std::to_string(flips));
        send(board);
        return line;
      }
      send(errorToString(flips));
    }
  } while (true);
}

std::string RemotePlayer::get() const {
  if (!_inputBuffer.size()) {
    const auto bytes = read_until(_socket, _inputBuffer, "\n", _error);
    if (_error && _error != error::eof) opFailed("read");
    if (_debug) std::cout << "### get - read bytes: " << bytes << ", buf: " << _inputBuffer.size() << '\n';
  }
  std::string line;
  std::getline(_input, line); // strips final '\n'
  if (_debug) std::cout << "### get - line: " << line << '\n';
  return line;
}

void RemotePlayer::send(const std::string& msg) const {
  if (_debug) std::cout << "### about to send: " << msg << '\n';
  write(_socket, buffer(msg + '\n'), _error);
  if (_error) opFailed("write");
}

void RemotePlayer::opFailed(const char* msg) const {
  std::cout << msg << " failed: " << _error.message() << '\n';
  exit(1);
}

} // namespace othello
