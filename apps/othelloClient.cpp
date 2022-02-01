#include "OthelloClient.h"

#include <random>

namespace othello {

using namespace boost::asio;
using ip::tcp;

OthelloClient::OthelloClient(int argc, char** argv) : _socket(_service), _input(&_inputBuffer) {
  for (auto i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-d")
      _debug = true;
    else if (arg == "-p")
      _printBoard = true;
    else if (arg == "-r")
      _random = true;
    else
      usage(argv[0], arg);
  }
  _socket.connect(tcp::endpoint(ip::address::from_string("127.0.0.1"), 1234));
}

void OthelloClient::begin() {
  send(_printBoard ? "printBoards" : "");
  auto turn = 0;
  auto endGame = false;
  do {
    // print board position after my last move
    if (turn) printBoard();
    auto moves = get();
    if (moves == "end") {
      endGame = true;
      moves = get();
    }
    if (!moves.empty()) {
      if (!turn) std::swap(_myColor, _serverColor); // server made first move so swap colors
      turn += moves.length() / 2;
      out(_serverColor, turn) << (moves.length() > 2 ? "moves were: " : "move was: ") << movesToString(moves) << '\n';
      // print board position after server's last move
      printBoard();
    } else if (!turn) {
      // print initial board position
      printBoard();
    } else if (!endGame)
      out(_serverColor) << "had no valid moves - skipping turn\n";
  } while (!endGame && makeMove(++turn));
  if (endGame) std::cout << "Game Over\n";
}

bool OthelloClient::makeMove(int turn) {
  static std::random_device rd;
  static std::mt19937 gen(rd());

  do {
    std::string line;
    if (_random) {
      send("v");
      if (const auto validMoves = get(); validMoves.length() > 2) {
        std::uniform_int_distribution<> dis(0, validMoves.length() / 2 - 1);
        line = validMoves.substr(dis(gen) * 2, 2);
      } else
        line = validMoves.substr(0, 2);
      assert(line.length() == 2);
      out(_myColor, turn) << "making random move at: " << line << '\n';
    } else {
      out(_myColor, turn) << "enter move (a1, b2, ... v=show valid moves, q=quit): ";
      std::cout.flush();
      std::getline(std::cin, line);
    }
    if (!line.empty()) {
      send(line);
      if (line == "q") return false;
      if (line == "v")
        std::cout << movesToString(get()) << '\n';
      else {
        const auto r = get();
        if (std::all_of(r.begin(), r.end(), ::isdigit)) {
          std::cout << "  ok - flipped " << r << " piece" << (r == "1" ? "" : "s") << '\n';
          return true;
        }
        std::cout << "  error - server returned: " << r << '\n';
      }
    }
  } while (true);
}

std::ostream& OthelloClient::out(const std::string& color, std::optional<int> turn) {
  std::cout << ">>> " << color << " ";
  if (turn) return std::cout << "(turn " << *turn << ") - ";
  return std::cout;
}

void OthelloClient::printBoard() {
  if (_printBoard) {
    const auto board = get();
    assert(board.length() == 64);
    auto black = 0, white = 0;
    std::cout << "\n   a b c d e f g h\n"
              << " +----------------\n";
    for (auto row = 0; row < 8; ++row) {
      std::cout << row + 1 << '|';
      for (auto col = 0; col < 8; ++col) {
        const auto c = board[row * 8 + col];
        if (c == '*') ++black;
        if (c == 'o') ++white;
        std::cout << ' ' << c;
      }
      if (row < 7) std::cout << '\n';
    }
    std::cout << "  Black(*): " << black << ", White(o): " << white << "\n\n";
  }
}

std::string OthelloClient::movesToString(const std::string& moves) {
  std::string result;
  for (size_t i = 0; i < moves.length(); i += 2) result += (i ? ", " : "") + moves.substr(i, 2);
  return result;
}

void OthelloClient::usage(const char* program, const std::string& arg) {
  const auto file = std::filesystem::path(program).stem().string();
  std::cerr << file << ": unrecognized option " << arg << "\nusage: " << file << " [-d] [-p] [-r]\n"
            << "  -d: show all messages sent and received from server\n"
            << "  -p: print board before and after each move\n"
            << "  -r: make a random move instead of waiting for user input\n";
  exit(1);
}

std::string OthelloClient::get() {
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

void OthelloClient::send(const std::string& msg) {
  if (_debug) std::cout << "### send - msg: " << msg << '\n';
  write(_socket, buffer(msg + '\n'), _error);
  if (_error) opFailed("write");
}

void OthelloClient::opFailed(const char* msg) {
  std::cout << msg << " failed: " << _error.message() << '\n';
  exit(1);
}

} // namespace othello
