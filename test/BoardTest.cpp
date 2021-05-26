#include <strstream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "Board.h"

using ::testing::StartsWith;

class BoardTest : public ::testing::Test {
protected:
  Board board;
};

TEST_F(BoardTest, BoardEnumHasExpectedValues) {
  EXPECT_EQ(board.Empty, 0);
  EXPECT_EQ(board.Black, 1);
  EXPECT_EQ(board.White, 2);
  EXPECT_EQ(board.Flip, 3);
}

TEST_F(BoardTest, PrintEmptyBoard) {
  std::ostrstream os;
  os << board;  
  EXPECT_THAT(os.str(), StartsWith("he"));
}
