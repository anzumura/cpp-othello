#include "Board.h"

#include <strstream>
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace othello {

using ::testing::StartsWith;

class BoardTest : public ::testing::Test {
 protected:
  void set(int emptyRows, const std::string& initialLayout) { board = Board(initialLayout, emptyRows * Board::RowSize); }
  void set(const std::string& initialLayout) { board = Board(initialLayout); }
  void check(int emptyRows, const std::string& expectedLayout) const {
    std::string expected(emptyRows * Board::RowSize, '.');
    check(expected + expectedLayout);
  }
  void check(const std::string& expected) const {
    EXPECT_EQ(board.toString(), expected + std::string(Board::BoardSize - expected.length(), '.'));
  }
  Board board;
};

TEST_F(BoardTest, BoardSize) {
  // 'board' should be 16 bytes (128 bits)
  EXPECT_EQ(sizeof(board), 16);
  // 'long long' should be 8 bytes (64 bits)
  EXPECT_EQ(sizeof(1LL), 8);
}

TEST_F(BoardTest, Scores) {
  // Initial score
  EXPECT_EQ(board.blackCount(), 2);
  EXPECT_EQ(board.whiteCount(), 2);
  // place one piece and then check scores again
  ASSERT_EQ(board.set("d3", Board::Color::Black), 1);  
  EXPECT_EQ(board.blackCount(), 4);
  EXPECT_EQ(board.whiteCount(), 1);
}

TEST_F(BoardTest, ValidMoves) {
  auto blackMoves = board.validMoves(Board::Color::Black);
  std::vector<std::string> expectedBlackMoves = {"d3", "c4", "f5", "e6"};
  ASSERT_EQ(blackMoves, expectedBlackMoves);
  auto whiteMoves = board.validMoves(Board::Color::White);
  std::vector<std::string> expectedWhiteMoves = {"e3", "f4", "c5", "d6"};
  ASSERT_EQ(whiteMoves, expectedWhiteMoves);
  set("\
........\
.xxxxxx.\
.xoooox.\
.xo.xox.\
.xox.ox.\
.xoooox.\
.xxxxxx.\
........");
  auto moves = board.validMoves(Board::Color::White);
  EXPECT_EQ(moves.size(), 30);
}

TEST_F(BoardTest, ValidMovesWithArrays) {
  Board::Moves moves;
  Board::Boards boards;
  auto result = board.validMoves(Board::Color::Black, moves, boards);
  ASSERT_EQ(result, 4);
  EXPECT_EQ(moves[0], 19);
  EXPECT_EQ(moves[1], 26);
  EXPECT_EQ(moves[2], 37);
  EXPECT_EQ(moves[3], 44);
  board = boards[0]; check(2, "\
...x....\
...xx...\
...xo");
  board = boards[1]; check(3, "\
..xxx...\
...xo");
  board = boards[2]; check(3, "\
...ox...\
...xxx");
  board = boards[3]; check(3, "\
...ox...\
...xx...\
....x");
}

TEST_F(BoardTest, ToStream) {
  auto expected = "\
+-+-----------------+-+\n\
| | A B C D E F G H | |\n\
+-+-----------------+-+\n\
|1| . . . . . . . . |1|\n\
|2| . . . . . . . . |2|\n\
|3| . . . . . . . . |3|\n\
|4| . . . o x . . . |4|\n\
|5| . . . x o . . . |5|\n\
|6| . . . . . . . . |6|\n\
|7| . . . . . . . . |7|\n\
|8| . . . . . . . . |8|\n\
+-+-----------------+-+\n\
| | A B C D E F G H | |\n\
+-+-----------------+-+\n\
Score - Black(x): 2, White(o): 2\n";
  std::ostrstream os;
  os << board;
  EXPECT_STREQ(os.str(), expected);
}

TEST_F(BoardTest, ToString) {
  check(3, "\
...ox...\
...xo");
}

TEST_F(BoardTest, FlipUp) {
  ASSERT_EQ(board.set("d6", Board::Color::White), 1);
  check(3, "\
...ox...\
...oo...\
...o");
  // test flip up boundry position
  auto moves = std::array { std::make_pair("o", 1), std::make_pair("x", 0) };
  for (const auto& m : moves) {
    set(std::string("x.......") + m.first);
    ASSERT_EQ(board.set("a3", Board::Color::Black), m.second);
  }
}

TEST_F(BoardTest, FlipDown) {
  ASSERT_EQ(board.set("d3", Board::Color::Black), 1);
  check(2, "\
...x....\
...xx...\
...xo");
  // test flip down boundry position
  auto moves = std::array { std::make_pair("o", 1), std::make_pair("x", 0) };
  for (const auto& m : moves) {
    set(6, std::string("\
.......x\
.......") + m.first);
    ASSERT_EQ(board.set("h6", Board::Color::White), m.second);
  }
}

TEST_F(BoardTest, FlipLeft) {
  ASSERT_EQ(board.set("f4", Board::Color::White), 1);
  check(3, "\
...ooo..\
...xo");
  // test flip left boundry position
  set("xo");
  ASSERT_EQ(board.set("c1", Board::Color::Black), 1);
  set("oo");
  ASSERT_EQ(board.set("c1", Board::Color::Black), 0);
}

TEST_F(BoardTest, FlipRight) {
  ASSERT_EQ(board.set("c5", Board::Color::White), 1);
  check(3, "\
...ox...\
..ooo");
  // test flip right boundry position
  set(7, "......ox");
  ASSERT_EQ(board.set("f8", Board::Color::Black), 1);
  set(7, "......oo");
  ASSERT_EQ(board.set("f8", Board::Color::Black), 0);
}

TEST_F(BoardTest, FlipUpLeft) {
  ASSERT_EQ(board.set("e6", Board::Color::Black), 1);
  ASSERT_EQ(board.set("f6", Board::Color::White), 1);
  check(3, "\
...ox...\
...xo...\
....xo");
}

TEST_F(BoardTest, FlipUpRight) {
  ASSERT_EQ(board.set("d6", Board::Color::White), 1);
  ASSERT_EQ(board.set("c6", Board::Color::Black), 1);
  check(3, "\
...ox...\
...xo...\
..xo");
}

TEST_F(BoardTest, FlipDownLeft) {
  ASSERT_EQ(board.set("e3", Board::Color::White), 1);
  ASSERT_EQ(board.set("f3", Board::Color::Black), 1);
  check(2, "\
....ox..\
...ox...\
...xo");
}

TEST_F(BoardTest, FlipDownRight) {
  ASSERT_EQ(board.set("d3", Board::Color::Black), 1);
  ASSERT_EQ(board.set("c3", Board::Color::White), 1);
  check(2, "\
..ox....\
...ox...\
...xo");
}

TEST_F(BoardTest, MultipleFlipsDown) {
  for (int i = 0; i < Board::RowSizeMinusTwo; ++i) {
    set(i + 1, "\
...xxx..\
..ooooo");
    std::vector<std::string> moves = { "c", "d", "e", "f", "g" };
    for(auto& m : moves) m += std::to_string(i + 1);
    ASSERT_EQ(board.validMoves(Board::Color::White), moves);
    ASSERT_EQ(board.set(moves[2], Board::Color::White), 3);
    check(i, "\
....o...\
...ooo..\
..ooooo");
  }
}

TEST_F(BoardTest, MultipleFlipsUp) {
  for (int i = 0; i < Board::RowSizeMinusTwo; ++i) {
    set(i, "\
..xxxxx.\
...ooo");
    std::vector<std::string> moves = { "c", "d", "e", "f", "g" };
    for(auto& m : moves) m += std::to_string(i + 3);
    ASSERT_EQ(board.validMoves(Board::Color::Black), moves);
    ASSERT_EQ(board.set(moves[2], Board::Color::Black), 3);
    check(i, "\
..xxxxx.\
...xxx..\
....x");
  }
}

TEST_F(BoardTest, MultipleFlipsLeft) {
  for (int i = 0; i < Board::RowSizeMinusTwo; ++i) {
    auto f = [i](const char* s){
      std::string result(i, '.');
      result.append(s);
      return result + std::string(Board::RowSizeMinusTwo - 1 - i, '.');
    };
    set(
      f("o..") +
      f("ox.") +
      f("ox.") +
      f("ox.") +
      f("o"));
    std::vector<std::string> moves;
    for (int j = 1; j < Board::RowSizeMinusTwo; ++j) moves.emplace_back(std::string(1, 'c' + i) + std::to_string(j));
    ASSERT_EQ(board.validMoves(Board::Color::White), moves);
    ASSERT_EQ(board.set(moves[2], Board::Color::White), 3);
    check(
      f("o..") +
      f("oo.") +
      f("ooo") +
      f("oo.") +
      f("o"));
  }
}

TEST_F(BoardTest, MultipleFlipsRight) {
  for (int i = 0; i < Board::RowSizeMinusTwo; ++i) {
    auto f = [i](const char* s){
      std::string result(i, '.');
      result.append(s);
      return result + std::string(Board::RowSizeMinusTwo - 1 - i, '.');
    };
    set(
      f("..x") +
      f(".ox") +
      f(".ox") +
      f(".ox") +
      f("..x"));
    std::vector<std::string> moves;
    for (int j = 1; j < Board::RowSizeMinusTwo; ++j) moves.emplace_back(std::string(1, 'a' + i) + std::to_string(j));
    ASSERT_EQ(board.validMoves(Board::Color::Black), moves);
    ASSERT_EQ(board.set(moves[2], Board::Color::Black), 3);
    check(
      f("..x") +
      f(".xx") +
      f("xxx") +
      f(".xx") +
      f("..x"));
  }
}

TEST_F(BoardTest, FlipHittingRightEdge) {
  set(1, "\
......oo\
xooooo.o\
......o.\
......x");
  ASSERT_EQ(board.set("g3", Board::Color::Black), 6);
  check(1, "\
......oo\
xxxxxxxo\
......x.\
......x");
}

TEST_F(BoardTest, FlipHittingBottomEdge) {
  set("\
..o.....\
..x.....\
..x.....\
..x.....\
..x.....\
..x.....\
.x.xxxxo\
.xx.....");
  ASSERT_EQ(board.set("c7", Board::Color::White), 9);
  check("\
..o.....\
..o.....\
..o.....\
..o.....\
..o.....\
..o.....\
.xoooooo\
.xx.....");
}

TEST_F(BoardTest, FlipHittingLeftEdge) {
  set("\
x......x\
.o.....o\
..o....o\
...o...o\
....o..o\
.....o.o\
......oo\
ooooooo.");
  ASSERT_EQ(board.set("h8", Board::Color::Black), 12);
  check("\
x......x\
.x.....x\
..x....x\
...x...x\
....x..x\
.....x.x\
......xx\
ooooooox");
}

TEST_F(BoardTest, FlipHittingTopEdge) {
  set("\
..o..x..\
x.o.o...\
.ooo....\
xo.oooox\
..oo....\
..o.o...\
..o..o..\
..x...x.");
  ASSERT_EQ(board.set("c4", Board::Color::Black), 14);
  set("\
..o..x..\
x.o.x...\
.xox....\
xxxxxxxx\
..xx....\
..x.x...\
..x..x..\
..x...x.");
}

TEST_F(BoardTest, SetFailsForBadRowOrColumn) {
  for (auto v : Board::Colors) {
    // bad lengths
    EXPECT_EQ(board.set("", v), Board::BadLength);
    EXPECT_EQ(board.set("f", v), Board::BadLength);
    EXPECT_EQ(board.set("f44", v), Board::BadLength);
    // bad rows
    EXPECT_EQ(board.set("f0", v), Board::BadRow);
    EXPECT_EQ(board.set("f9", v), Board::BadRow);
    EXPECT_EQ(board.set("fa", v), Board::BadRow);
    // bad columns
    EXPECT_EQ(board.set("F4", v), Board::BadColumn);
    EXPECT_EQ(board.set("24", v), Board::BadColumn);
    EXPECT_EQ(board.set("i4", v), Board::BadColumn);
    // occupied spaces
    EXPECT_EQ(board.set("d4", v), Board::BadCell);
    EXPECT_EQ(board.set("e4", v), Board::BadCell);
    EXPECT_EQ(board.set("d5", v), Board::BadCell);
    EXPECT_EQ(board.set("e5", v), Board::BadCell);
  }
}

}  // namespace othello
