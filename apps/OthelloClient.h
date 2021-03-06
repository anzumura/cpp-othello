#pragma once

#include <filesystem>
#include <iostream>
#include <optional>

#include <boost/asio.hpp>

namespace othello {

class OthelloClient {
public:
  OthelloClient(int argc, char** argv);
  void begin();
private:
  bool makeMove(size_t turn);

  std::ostream& out(const std::string& color, std::optional<size_t> turn = {});
  void printBoard();
  std::string movesToString(const std::string&);
  void usage(const char* program, const std::string& arg);

  // helper functions for sending and receiving data
  std::string get();
  void send(const std::string&);
  void opFailed(const char*);

  bool _debug = false;
  bool _printBoard = false;
  bool _random = false;
  std::string _myColor = "Black";
  std::string _serverColor = "White";
  boost::asio::io_service _service;
  boost::asio::ip::tcp::socket _socket;
  boost::asio::streambuf _inputBuffer;
  std::istream _input;
  boost::system::error_code _error;
};

} // namespace othello
