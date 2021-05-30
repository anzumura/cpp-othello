#include "Player.h"

#include <cctype>
#include <iostream>
#include <random>
#include <string>

namespace othello {

std::unique_ptr<Player> Player::createPlayer(Board::Color color) {
  do {
    std::cout << "Choose type for " << color << " player (h=human, c=computer): ";
    std::string line;
    std::getline(std::cin, line);
    if (line.length() == 1) {
      if (line[0] == 'h') return std::make_unique<HumanPlayer>(color);
      if (line[0] == 'c') return std::make_unique<ComputerPlayer>(color);
    }
  } while (true);
}

bool Player::move(Board& board) const {
  std::cout << std::endl << board << std::endl;
  return makeMove(board);
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
