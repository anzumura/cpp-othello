#include <strstream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "Board.h"

using ::testing::StartsWith;

class BoardTest : public ::testing::Test {
protected:
  Board board;
};

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
  auto expected = "\
........\
........\
........\
...ox...\
...xo...\
........\
........\
........";
  EXPECT_STREQ(board.toString().c_str(), expected);
}
