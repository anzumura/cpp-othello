#include "Player.h"

#include <cctype>
#include <chrono>
#include <iostream>
#include <random>
#include <string>

namespace othello {

std::unique_ptr<Player> Player::createPlayer(Board::Color color) {
  char x = getChar(std::string("Choose type for ") + othello::toString(color) + " player (h=human, c=computer)",
    [](char x){ return x == 'h' || x == 'c'; });
  if (x == 'h') return std::make_unique<HumanPlayer>(color);
  x = getChar("  Enter computer type (0=random, 1-9=moves to search)",
    [](char x){ return x >= '0' && x <= '9'; });
  return std::make_unique<ComputerPlayer>(color, x - '0');
}

char Player::getChar(const std::string& msg, bool pred(char)) {
  std::string line;
  do {
    std::cout << msg << ": ";
    std::getline(std::cin, line);
  } while(line.length() != 1 || !pred(line[0]));
  return line[0];
}

bool Player::move(Board& board) const {
  std::cout << std::endl << board << std::endl;
  auto start = std::chrono::high_resolution_clock::now();
  auto result = makeMove(board);
  totalTime += std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::high_resolution_clock::now() - start);
  return result;
}

void Player::printTotalTime() const {
  std::cout << "Total time for " << toString() << ": "
    << std::chrono::duration_cast<std::chrono::microseconds>(totalTime).count() / 1'000'000.0 << " seconds\n";
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
        for (const auto& m : moves) std::cout << " " << m;
        std::cout << std::endl;
      }
    } else {
      auto result = board.set(line, color);
      if (result > 0) return true;
      std::cout << "  invalid move: " << line << std::endl;
      std::cout << "  please enter a location (eg 'a1' or 'h8'), 'q' to quit or 'v' to print valid moves\n";
    }
  } while (true);
}

bool ComputerPlayer::makeMove(Board& board) const {
  static std::random_device rd;
  static std::mt19937 gen(rd());

  auto moves = board.validMoves(color);
  int move = 0;
  if (moves.size() > 1) {
    std::uniform_int_distribution<> dis(0, moves.size() - 1);
    move = dis(gen);
  }
  int result = board.set(moves[move], color);
  std::cout << color << " played at: " << moves[move] << " (" << result
    << " flip" << (result > 1 ? "s" : "") << ")\n";
  return true;
}

}  // namespace othello
