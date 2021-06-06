#include "Player.h"

#include <cassert>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <string>

#include "Score.h"

namespace othello {

bool Player::move(Board& board, bool tournament) const {
  assert(board.hasValidMoves(color));
  if (!tournament) std::cout << std::endl << board << std::endl;
  auto start = std::chrono::high_resolution_clock::now();
  auto result = makeMove(board);
  totalTime += std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
  return result;
}

void Player::printTotalTime() const {
  std::cout << "Total time for " << toString() << ": " << std::fixed << std::setprecision(6)
            << totalTime.count() / 1'000'000'000.0 << " seconds\n";
}

bool HumanPlayer::makeMove(Board& board) const {
  do {
    std::cout << "enter " << color << "'s move: ";
    std::string line;
    std::getline(std::cin, line);
    if (line.length() == 1) {
      if (line[0] == 'q') return false;
      if (line[0] == 'v') {
        auto moves = board.validMoves(color);
        std::cout << "  valid moves are:";
        for (const auto& m : moves)
          std::cout << " " << m;
        std::cout << std::endl;
      }
    } else {
      auto result = board.set(line, color);
      if (result > 0) return true;
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

bool ComputerPlayer::makeMove(Board& board) const {
  static std::random_device rd;
  static std::mt19937 gen(rd());

  auto moves = _search == 0 ? board.validMoves(color) : findMove(board);
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
  return true;
}

std::vector<std::string> ComputerPlayer::findMove(const Board& board) const {
  Board::Moves moves;
  Board::Boards boards;
  auto totalMoves = board.validMoves(color, moves, boards);
  // return more than one position if moves have the same score
  std::vector<int> bestMoves;
  int best = -Score::Win;
  for (int i = 0; i < totalMoves; ++i) {
    int score = _search == 1 ? _score->score(boards[i], color) : minmax(boards[i], _search - 1, Board::opColor(color));
    if (score > best) {
      best = score;
      bestMoves.clear();
      bestMoves.push_back(moves[i]);
    } else if (score == best)
      bestMoves.push_back(moves[i]);
  }
  std::vector<std::string> results;
  for (auto p : bestMoves)
    results.push_back(Board::posToString(p));
  return results;
}

int ComputerPlayer::minmax(const Board& board, int depth, Board::Color turnColor) const {
  Board::Boards boards;
  auto totalMoves = board.validMoves(turnColor, boards);
  // if no valid moves then return score of the given board
  if (totalMoves == 0) return _score->score(board, color);
  // special case for depth 1 since we already calculated all the child boards when checking for valid moves
  if (depth == 1) {
    if (turnColor == color) {
      int best = -Score::Win;
      for (int i = 0; i < totalMoves; ++i) {
        int score = _score->score(boards[i], color);
        if (score > best) best = score;
      }
      return best;
    }
    int best = Score::Win;
    for (int i = 0; i < totalMoves; ++i) {
      int score = _score->score(boards[i], color);
      if (score < best) best = score;
    }
    return best;
  }
  // usual minmax algorithm, call recursively with depth -1 and opposite color
  if (turnColor == color) {
    int best = -Score::Win;
    for (int i = 0; i < totalMoves; ++i) {
      int score = minmax(boards[i], depth - 1, Board::opColor(color));
      if (score > best) best = score;
    }
    return best;
  }
  int best = Score::Win;
  for (int i = 0; i < totalMoves; ++i) {
    int score = minmax(boards[i], depth - 1, color);
    if (score < best) best = score;
  }
  return best;
}

} // namespace othello
