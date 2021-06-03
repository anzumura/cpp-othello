#include "Board.h"
#include "Score.h"

#include <array>
#include <string>
#include <utility>
#include "gtest/gtest.h"

namespace othello {

class ScoreTest : public ::testing::Test {
 protected:
  void set(int emptyRows, const std::string& initialLayout) { board = Board(initialLayout, emptyRows * Board::RowSize); }
  void set(const std::string& initialLayout) { board = Board(initialLayout); }
  Board board;
};

TEST_F(ScoreTest, InitialPosition) {
  for (auto c : Board::Colors)
    EXPECT_EQ(Score::score(board, c), 0);
}

TEST_F(ScoreTest, AfterOneFlip) {
  board.set("d3", Board::Color::Black); // flips 1 resulting in 4 black and 1 white
  EXPECT_EQ(Score::score(board, Board::Color::Black), 3);
  EXPECT_EQ(Score::score(board, Board::Color::White), -3);
}

TEST_F(ScoreTest, Corners) {
  // use boards with at least one valid move in order to avoid 'Win' scores
  std::array corners {
    std::make_pair(0, "\
*.......\
........\
..*o"),
    std::make_pair(0, "\
.......*\
........\
..*o"),
    std::make_pair(5, "\
..*o....\
........\
*"),
    std::make_pair(5, "\
..*o....\
........\
.......*")
  };
  for (auto c : corners) {
    set(c.first, c.second);
    EXPECT_EQ(Score::score(board, Board::Color::Black), Score::Corner);
    EXPECT_EQ(Score::score(board, Board::Color::White), -Score::Corner);
  }
}

TEST_F(ScoreTest, SafeHorizontalEdge) {
  set("\
oo......\
........\
..*o");
  EXPECT_EQ(Score::score(board, Board::Color::White), Score::Corner + Score::SafeEdge);
  set("\
oo....oo\
........\
..*o");
  EXPECT_EQ(Score::score(board, Board::Color::White), 2 * (Score::Corner + Score::SafeEdge));
  set("\
oooo****\
........\
..*o");
  EXPECT_EQ(Score::score(board, Board::Color::White), 0);
  set(5, "\
....o*..\
........\
*o*o*o*o");
  EXPECT_EQ(Score::score(board, Board::Color::White), 0);
}

TEST_F(ScoreTest, SafeVerticalEdge) {
  set("\
.......o\
.......o\
..*o");
  EXPECT_EQ(Score::score(board, Board::Color::White), Score::Corner + Score::SafeEdge);
  set("\
.......o\
.......o\
........\
........\
..*o....\
........\
.......o\
.......o");
  EXPECT_EQ(Score::score(board, Board::Color::White), 2 * (Score::Corner + Score::SafeEdge));
  set("\
.......*\
.......*\
.......*\
.......*\
.......o\
.......o\
.......o\
..*o...o");
  EXPECT_EQ(Score::score(board, Board::Color::White), 0);
  set("\
o.......\
o.......\
*.......\
*...*o..\
o.......\
o.......\
*.......\
*");
  EXPECT_EQ(Score::score(board, Board::Color::White), 0);
}

}  // namespace othello