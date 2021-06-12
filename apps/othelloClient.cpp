#include <filesystem>
#include <iostream>

#include <boost/asio.hpp>

int testAfterFree() {
  int* testInt = new int{7};
  delete testInt;
  return *testInt - 7;
}

char* testAfterStackChar;
void testAfterStack() {
  char stackChar = 'a';
  *testAfterStackChar = stackChar;
}

using namespace boost::asio;
using ip::tcp;

class OthelloClient {
public:
  OthelloClient() : _socket(_service) { _socket.connect(tcp::endpoint(ip::address::from_string("127.0.0.1"), 1234)); }
  void begin(bool printBoards, bool randomMoves) {
    send(printBoards ? "printBoards" : "");
    int turn = 0;
    bool endGame = false;
    do {
      // print board position after my last move
      if (turn && printBoards) printBoard();
      auto moves = get();
      if (moves == "end") {
        endGame = true;
        moves = get();
      }
      if (!moves.empty()) {
        // print board position after server's last move
        if (printBoards) printBoard();
        if (!turn) setColors(true);
        turn += moves.length() / 2;
        out(_serverColor, turn) << (moves.length() > 2 ? "moves were: ": "move was: ") << movesToString(moves) << '\n';
      } else if (!turn) {
        // print initial board position
        if (printBoards) printBoard();
        setColors(false);
      } else if (!endGame)
        out(_serverColor) << "had no valid moves - skipping turn\n";
    } while (!endGame && makeMove(++turn, randomMoves));
  }

private:
  bool makeMove(int turn, bool randomMoves) {
    do {
      std::string line;
      if (randomMoves) {
        send("v");
        auto validMoves = get();
        line = validMoves.substr(0, 2);
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
          std::string r = get();
          if (std::all_of(r.begin(), r.end(), ::isdigit)) {
            std::cout << "  ok - flipped " << r << " piece" << (r == "1" ? "" : "s") << '\n';
            return true;
          }
          std::cout << "  error - server returned: " << r << '\n';
        }
      }
    } while (true);
  }

  std::ostream& out(const std::string& color, std::optional<int> turn = {}) {
    std::cout << ">>> " << color;
    if (turn)
      return std::cout << " (turn " << *turn << ") - ";
    return std::cout << " ";
  }

  void setColors(bool serverMadeFirstMove) {
    if (serverMadeFirstMove) {
      _myColor = "White";
      _serverColor = "Black";
    } else {
      _myColor = "Black";
      _serverColor = "White";
    }
  }

  void printBoard() {
    const auto board = get();
    std::cout << board << '\n';
  }

  std::string movesToString(const std::string& moves) {
    std::string result;
    for (int i = 0; i < moves.length(); i += 2)
      result += (i ? ", " : "") + moves.substr(i, 2);
    return result;
  }

  std::string get() {
    streambuf buf;
    read_until(_socket, buf, "\n", _error);
    if (_error && _error != error::eof) opFailed("read");
    std::istream input(&buf);
    std::string line;
    std::getline(input, line); // strips final '\n'
    return line;
  }
  void send(const std::string& msg) {
    write(_socket, buffer(msg + '\n'), _error);
    if (_error) opFailed("write");
  }
  void opFailed(const char* msg) {
    std::cout << msg << " failed: " << _error.message() << '\n';
    exit(1);
  }

  std::string _myColor;
  std::string _serverColor;
  io_service _service;
  tcp::socket _socket;
  boost::system::error_code _error;
};

void usage(const char* program, const std::string& arg) {
  auto file = std::filesystem::path(program).stem().string();
  std::cerr << file << ": unrecognized option " << arg;
  std::cerr << "\nusage: " << file << " [-p] [-r]\n";
  exit(1);
}

int main(int argc, char** argv) {
  // Uncomment these functions to test Address Sanitizer:
  // - testAfterStack: gets caught before address sanitizer with 'Trace/BPT trap: 5'
  // - testAfterFree: seems to work properly, i.e., no errors unless compiled with sanitizer flags
  // testAfterStack(); *testAfterStackChar = 'b';
  // return testAfterFree();
  bool printBoards = false, randomMoves = false;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "-p")
      printBoards = true;
    else if (arg == "-r")
      randomMoves = true;
    else
      usage(argv[0], arg);
  }
  OthelloClient().begin(printBoards, randomMoves);
  return 0;
}
