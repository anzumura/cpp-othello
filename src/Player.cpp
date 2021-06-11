#include <othello/Player.h>
#include <othello/Score.h>

#include <iomanip>
#include <random>
#include <sstream>

namespace othello {

Player::Move Player::move(Board& board, bool tournament, Move prevMove) const {
  assert(board.hasValidMoves(color));
  if (!tournament) std::cout << std::endl << board << std::endl;
  int flips = 0;
  auto start = std::chrono::high_resolution_clock::now();
  auto move = makeMove(board, prevMove, flips);
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
  case Board::BadLength:
    return "location must be 2 characters";
  case Board::BadColumn:
    return "column must be a value from 'a' to 'h'";
  case Board::BadRow:
    return "row must be a value from '1' to '8'";
  case Board::BadCell:
    return "cell already occupied";
  }
  return "must flip at least one piece";
}

Player::Move HumanPlayer::makeMove(Board& board, Move, int& flips) const {
  do {
    std::cout << "enter " << color << "'s move: ";
    std::string line;
    std::getline(std::cin, line);
    if (line.length() == 1) {
      if (line[0] == 'q') return {};
      if (line[0] == 'v') {
        auto moves = board.validMoves(color);
        std::cout << "  valid moves are:";
        for (const auto& m : moves)
          std::cout << " " << m;
        std::cout << std::endl;
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
  std::locale loc("en_US.UTF-8");
  ss.imbue(loc);
  ss << " search=" << _search << " (score called " << _totalScoreCalls << ")";
  return ss.str();
}

Player::Move ComputerPlayer::makeMove(Board& board, Move, int& flips) const {
  static std::random_device rd;
  static std::mt19937 gen(rd());

  auto moves = _search == 0 ? board.validMoves(color) : findMoves(board);
  assert(!moves.empty());
  int move = 0;
  if (_random && moves.size() > 1) {
    std::uniform_int_distribution<> dis(0, moves.size() - 1);
    move = dis(gen);
  }
  flips = board.set(moves[move], color);
  return moves[move];
}

std::vector<std::string> ComputerPlayer::findMoves(const Board& board) const {
  Board::Boards boards;
  Board::Positions positions;
  const int moves = board.validMoves(color, boards, positions);
  const int nextLevel = _search - 1;
  // return more than one position if moves have the same score
  Moves bestMoves;
  for (int i = 0, best = Min; i < moves; ++i)
    updateMoves(callMinMax(boards[i], nextLevel, opColor, moves, best, Max), i, best, bestMoves);
  // if there are multiple moves with the same score then only return ones with the best 'first move' score
  if (bestMoves.size() > 1) {
    int best = Min;
    std::vector<int> newBestMoves;
    for (auto i : bestMoves)
      updateMoves(callScore(boards[i]), i, best, newBestMoves);
    bestMoves = newBestMoves;
  }
  std::vector<std::string> results;
  for (auto i : bestMoves)
    results.emplace_back(Board::posToString(positions[i]));
  return results;
}

int ComputerPlayer::minMax(const Board& board, int depth, Board::Color turn, int prevMoves, int alpha, int beta) const {
  Board::Boards boards;
  const int moves = board.validMoves(turn, boards);
  const int nextLevel = depth - 1;
  // if no valid moves for current player then go to next level unless there were no valid moves
  // for previous level - in this case stop traversing and return score (by setting depth to 0)
  if (moves == 0) return callMinMax(board, prevMoves ? nextLevel : 0, Board::opColor(turn), 0, alpha, beta);
  // maximizing player
  if (turn == color) {
    int best = Min;
    for (int i = 0; i < moves && best < beta; ++i, alpha = std::max(alpha, best))
      best = std::max(best, callMinMax(boards[i], nextLevel, opColor, moves, alpha, beta));
    return best;
  }
  // minimizing player
  int best = Max;
  for (int i = 0; i < moves && best > alpha; ++i, beta = std::min(beta, best))
    best = std::min(best, callMinMax(boards[i], nextLevel, color, moves, alpha, beta));
  return best;
}

RemotePlayer::RemotePlayer(Board::Color c)
    : Player(c), _acceptor(_service, tcp::endpoint(tcp::v4(), Port)), _socket(_service) {}

using namespace boost::asio;

Player::Move RemotePlayer::makeMove(Board& board, Move prevMove, int& flips) const {
  if (!_socket.is_open()) {
    std::cout << color << ": waiting for initial connection on port " << Port << " ... ";
    std::cout.flush();
    _acceptor.accept(_socket);
    std::cout << "connected\n\n";
  }
  send(prevMove.value_or(""));
  do {
    if (std::string line = get(); line == "q")
      return {};
    else if (line == "v") {
      auto v = board.validMoves(color);
      std::string out;
      for (const auto& s : v)
        out += (out.empty() ? s : ", " + s);
      send(out);
    } else {
      flips = board.set(line, color);
      if (flips > 0) {
        send(std::to_string(flips));
        return line;
      }
      send(errorToString(flips));
    }
  } while (true);
}

std::string RemotePlayer::get() const {
  streambuf buf;
  read_until(_socket, buf, "\n", _error);
  if (_error && _error != error::eof) opFailed("read");
  std::istream input(&buf);
  std::string line;
  std::getline(input, line); // strips final '\n'
  return line;
}

void RemotePlayer::send(const std::string& msg) const {
  write(_socket, buffer(msg + '\n'), _error);
  if (_error) opFailed("write");
}

void RemotePlayer::opFailed(const char* msg) const {
  std::cout << msg << " failed: " << _error.message() << '\n';
  exit(1);
}

} // namespace othello
