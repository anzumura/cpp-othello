#include <strstream>

#include "Board.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::testing::StartsWith;

class BoardTest : public ::testing::Test {
 protected:
  void set(const char* initialLayout) { board = Board(initialLayout); }
  void check(const char* expectedLayout) {
    ASSERT_STREQ(board.toString().c_str(), expectedLayout);
  }
  Board board;
  std::array<BoardValue, 2> values = {BoardValue::White, BoardValue::Black};
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
  ASSERT_EQ(board.set("d3", BoardValue::Black), 1);  
  EXPECT_EQ(board.blackCount(), 4);
  EXPECT_EQ(board.whiteCount(), 1);
}

TEST_F(BoardTest, ValidMoves) {
  auto blackMoves = board.validMoves(BoardValue::Black);
  std::vector<std::string> expectedBlackMoves = {"d3","c4","f5","e6"};
  ASSERT_EQ(blackMoves, expectedBlackMoves);
  auto whiteMoves = board.validMoves(BoardValue::White);
  std::vector<std::string> expectedWhiteMoves = {"e3","f4","c5","d6"};
  ASSERT_EQ(whiteMoves, expectedWhiteMoves);
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
+-+-----------------+-+\n";
  std::ostrstream os;
  os << board;
  EXPECT_STREQ(os.str(), expected);
}

TEST_F(BoardTest, ToString) {
  check("\
........\
........\
........\
...ox...\
...xo...\
........\
........\
........");
}

TEST_F(BoardTest, FlipUp) {
  ASSERT_EQ(board.set("d6", BoardValue::White), 1);
  check("\
........\
........\
........\
...ox...\
...oo...\
...o....\
........\
........");
}

TEST_F(BoardTest, FlipDown) {
  ASSERT_EQ(board.set("d3", BoardValue::Black), 1);
  check("\
........\
........\
...x....\
...xx...\
...xo...\
........\
........\
........");
}

TEST_F(BoardTest, FlipLeft) {
  ASSERT_EQ(board.set("f4", BoardValue::White), 1);
  check("\
........\
........\
........\
...ooo..\
...xo...\
........\
........\
........");
}

TEST_F(BoardTest, FlipRight) {
  ASSERT_EQ(board.set("c5", BoardValue::White), 1);
  check("\
........\
........\
........\
...ox...\
..ooo...\
........\
........\
........");
}

TEST_F(BoardTest, FlipUpLeft) {
  ASSERT_EQ(board.set("e6", BoardValue::Black), 1);
  ASSERT_EQ(board.set("f6", BoardValue::White), 1);
  check("\
........\
........\
........\
...ox...\
...xo...\
....xo..\
........\
........");
}

TEST_F(BoardTest, FlipUpRight) {
  ASSERT_EQ(board.set("d6", BoardValue::White), 1);
  ASSERT_EQ(board.set("c6", BoardValue::Black), 1);
  check("\
........\
........\
........\
...ox...\
...xo...\
..xo....\
........\
........");
}

TEST_F(BoardTest, FlipDownLeft) {
  ASSERT_EQ(board.set("e3", BoardValue::White), 1);
  ASSERT_EQ(board.set("f3", BoardValue::Black), 1);
  check("\
........\
........\
....ox..\
...ox...\
...xo...\
........\
........\
........");
}

TEST_F(BoardTest, FlipDownRight) {
  ASSERT_EQ(board.set("d3", BoardValue::Black), 1);
  ASSERT_EQ(board.set("c3", BoardValue::White), 1);
  check("\
........\
........\
..ox....\
...ox...\
...xo...\
........\
........\
........");
}

TEST_F(BoardTest, MultipleFlipsDown) {
  set("\
........\
........\
........\
...xxx..\
..ooooo.\
........\
........\
........");
  ASSERT_EQ(board.set("e3", BoardValue::White), 3);
  check("\
........\
........\
....o...\
...ooo..\
..ooooo.\
........\
........\
........");
}

TEST_F(BoardTest, MultipleFlipsUp) {
  set("\
........\
........\
........\
..xxxxx.\
...ooo..\
........\
........\
........");
  ASSERT_EQ(board.set("e6", BoardValue::Black), 3);
  check("\
........\
........\
........\
..xxxxx.\
...xxx..\
....x...\
........\
........");
}

TEST_F(BoardTest, FlipHittingRightEdge) {
  set("\
........\
......oo\
xooooo.o\
......o.\
......x.\
........\
........\
........");
  ASSERT_EQ(board.set("g3", BoardValue::Black), 6);
  check("\
........\
......oo\
xxxxxxxo\
......x.\
......x.\
........\
........\
........");
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
  ASSERT_EQ(board.set("c7", BoardValue::White), 9);
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
  ASSERT_EQ(board.set("h8", BoardValue::Black), 12);
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
  ASSERT_EQ(board.set("c4", BoardValue::Black), 14);
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
  for (auto v : values) {
    // bad lengths
    EXPECT_EQ(board.set("", v), 0);
    EXPECT_EQ(board.set("f", v), 0);
    EXPECT_EQ(board.set("f44", v), 0);
    // bad rows
    EXPECT_EQ(board.set("f0", v), 0);
    EXPECT_EQ(board.set("f9", v), 0);
    EXPECT_EQ(board.set("fa", v), 0);
    // bad columns
    EXPECT_EQ(board.set("F4", v), 0);
    EXPECT_EQ(board.set("24", v), 0);
    EXPECT_EQ(board.set("i4", v), 0);
    // occupied spaces
    EXPECT_EQ(board.set("d4", v), 0);
    EXPECT_EQ(board.set("e4", v), 0);
    EXPECT_EQ(board.set("d5", v), 0);
    EXPECT_EQ(board.set("e5", v), 0);
  }
}
