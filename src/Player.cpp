#include <othello/Player.h>
#include <othello/Score.h>

#include <iomanip>
#include <random>
#include <sstream>

namespace othello {

Player::Move Player::move(Board& board, bool tournament, Move prevMove) const {
  assert(board.hasValidMoves(color));
  if (!tournament) std::cout << std::endl << board << std::endl;
  auto start = std::chrono::high_resolution_clock::now();
  auto result = makeMove(board, prevMove);
  totalTime += std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
  return result;
}

void Player::printTotalTime() const {
  std::cout << "Total time for " << toString() << ": " << std::fixed << std::setprecision(6)
            << totalTime.count() / 1'000'000'000.0 << " seconds\n";
}

Player::Move HumanPlayer::makeMove(Board& board, Move) const {
  do {
    std::cout << "enter " << color << "'s move: ";
    std::string line;
    std::getline(std::cin, line);
    if (line.length() == 1) {
      if (line[0] == 'q') return std::nullopt;
      if (line[0] == 'v') {
        auto moves = board.validMoves(color);
        std::cout << "  valid moves are:";
        for (const auto& m : moves)
          std::cout << " " << m;
        std::cout << std::endl;
      }
    } else {
      auto result = board.set(line, color);
      if (result > 0) return line;
      std::cout << "  invalid move: '" << line << "' - " << errorToString(result)
                << "\n  please enter a location (eg 'a1' or 'h8'), 'q' to quit or 'v' to print valid moves\n";
    }
  } while (true);
}

const char* HumanPlayer::errorToString(int result) {
  switch (result) {
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

Player::Move ComputerPlayer::makeMove(Board& board, Move) const {
  static std::random_device rd;
  static std::mt19937 gen(rd());

  auto moves = _search == 0 ? board.validMoves(color) : findMoves(board);
  assert(!moves.empty());
  int move = 0;
  if (_random && moves.size() > 1) {
    std::uniform_int_distribution<> dis(0, moves.size() - 1);
    move = dis(gen);
  }
  int result = board.set(moves[move], color);
  assert(result > 0);
  if (!_tournament)
    std::cout << color << " played at: " << moves[move] << " (" << result << " flip" << (result > 1 ? "s" : "")
              << ")\n";
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

} // namespace othello
