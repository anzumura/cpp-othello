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
  void begin() {
    do {
      std::string move = get();
      if (!move.empty()) std::cout << ">>> Server move was: " << move << "\n";
    } while (makeMove());
  }

private:
  bool makeMove() {
    do {
      std::cout << ">>> Enter move ('a1', 'b2', ...) 'v' to show valid moves or 'q' to quit: ";
      std::string line;
      std::getline(std::cin, line);
      if (!line.empty()) {
        send(line);
        if (line == "q") return false;
        if (line == "v")
          std::cout << get() << '\n';
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

  io_service _service;
  tcp::socket _socket;
  boost::system::error_code _error;
};

int main() {
  // Uncomment these functions to test Address Sanitizer:
  // - testAfterStack: gets caught before address sanitizer with 'Trace/BPT trap: 5'
  // - testAfterFree: seems to work properly, i.e., no errors unless compiled with sanitizer flags
  // testAfterStack(); *testAfterStackChar = 'b';
  // return testAfterFree();
  OthelloClient().begin();
  return 0;
}
