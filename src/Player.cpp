#include "Player.h"

#include <cassert>
#include <cctype>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

#include "Score.h"

namespace othello {

std::unique_ptr<Player> Player::createPlayer(Board::Color color) {
  char x = getChar(std::string("Choose type for ") + othello::toString(color) + " player (h=human, c=computer)",
                   [](char x) { return x == 'h' || x == 'c'; });
  if (x == 'h') return std::make_unique<HumanPlayer>(color);
  x = getChar("  Enter computer type (0=random, 1-9=moves to search)", [](char x) { return x >= '0' && x <= '9'; });
  return std::make_unique<ComputerPlayer>(color, x - '0');
}

char Player::getChar(const std::string& msg, bool pred(char)) {
  std::string line;
  do {
    std::cout << msg << ": ";
    std::getline(std::cin, line);
  } while (line.length() != 1 || !pred(line[0]));
  return line[0];
}

bool Player::move(Board& board) const {
  assert(board.hasValidMoves(color));
  std::cout << std::endl << board << std::endl;
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
  if (moves.size() > 1) {
    std::uniform_int_distribution<> dis(0, moves.size() - 1);
    move = dis(gen);
  }
  int result = board.set(moves[move], color);
  std::cout << color << " played at: " << moves[move] << " (" << result << " flip" << (result > 1 ? "s" : "") << ")\n";
  return true;
}

std::vector<std::string> ComputerPlayer::findMove(const Board& board) const {
  Board::Moves moves;
  Board::Boards boards;
  auto total = board.validMoves(color, moves, boards);
  int bestScore = -Score::Win - 1;
  std::vector<int> bestPositions;
  for (int i = 0; i < total; ++i) {
    int score = Score::score(boards[i], color);
    if (score > bestScore) {
      bestScore = score;
      bestPositions.clear();
      bestPositions.push_back(moves[i]);
    } else if (score == bestScore)
      bestPositions.push_back(moves[i]);
  }
  std::vector<std::string> results;
  for (auto p : bestPositions)
    results.push_back(Board::posToString(p));
  return results;
}

} // namespace othello
