#include "Score.h"
#include "Board.h"

#include "gtest/gtest.h"
#include <array>
#include <string>
#include <utility>

namespace othello {

class ScoreTest : public ::testing::Test {
protected:
  void set(int emptyRows, const std::string& initialLayout) { board = Board(initialLayout, emptyRows * Board::Rows); }
  void set(const std::string& initialLayout) { board = Board(initialLayout); }
  void check(int s) {
    for (auto c : Board::Colors)
      ASSERT_EQ(Score::score(board, c), c == Board::Color::Black ? s : -s) << "Color: " << c;
  }
  Board board;
};

TEST_F(ScoreTest, InitialPosition) {
  for (auto c : Board::Colors)
    EXPECT_EQ(Score::score(board, c), 0);
}

TEST_F(ScoreTest, Win) {
  set("*");
  check(Score::Win);
  set(std::string(64, '*'));
  check(Score::Win);
}

TEST_F(ScoreTest, Draw) {
  set(""); // board with no valid moves and equal counts
  check(0);
  set("*.o");
  check(0);
}

TEST_F(ScoreTest, AfterOneFlip) {
  board.set("d3", Board::Color::Black); // flips 1 resulting in 4 black and 1 white
  check(3);
}

TEST_F(ScoreTest, Corners) {
  // use boards with at least one valid move in order to avoid 'Win' scores
  std::array corners{std::make_pair(0, "\
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
.......*")};
  for (auto c : corners) {
    set(c.first, c.second);
    check(Score::Corner);
  }
}

TEST_F(ScoreTest, SafeHorizontalEdge) {
  set("\
**......\
........\
..*o");
  check(Score::Corner + Score::SafeEdge);
  set("\
**....**\
........\
..*o");
  check(2 * (Score::Corner + Score::SafeEdge));
  set("\
oooo****\
........\
..*o");
  check(0);
  set(5, "\
....o*..\
........\
*o*o*o*o");
  check(0);
}

TEST_F(ScoreTest, SafeVerticalEdge) {
  set("\
.......*\
.......*\
..*o");
  check(Score::Corner + Score::SafeEdge);
  set("\
.......*\
.......*\
........\
........\
..*o....\
........\
.......*\
.......*");
  check(2 * (Score::Corner + Score::SafeEdge));
  set("\
.......*\
.......*\
.......*\
.......*\
.......o\
.......o\
.......o\
..*o...o");
  check(0);
  set("\
o.......\
o.......\
*.......\
*...*o..\
o.......\
o.......\
*.......\
*");
  check(0);
}

TEST_F(ScoreTest, EdgeNextToTopEmptyCorners) {
  set("\
.*......\
........\
..*o");
  check(Score::EmptyCornerEdge);
  set("\
......*.\
........\
..*o");
  check(Score::EmptyCornerEdge);
  set("\
........\
*.......\
..*o");
  check(Score::EmptyCornerEdge);
  set("\
........\
.......*\
..*o");
  check(Score::EmptyCornerEdge);
  set("\
.*......\
.......*\
..*o");
  check(2 * Score::EmptyCornerEdge);
}

TEST_F(ScoreTest, EdgeNextToBottomEmptyCorners) {
  set(5, "\
..*o....\
........\
.*");
  check(Score::EmptyCornerEdge);
  set(5, "\
..*o....\
........\
......*");
  check(Score::EmptyCornerEdge);
  set(5, "\
..*o....\
*");
  check(Score::EmptyCornerEdge);
  set(5, "\
..*o....\
.......*");
  check(Score::EmptyCornerEdge);
  set(5, "\
..*o....\
.......*\
......*.");
  check(2 * Score::EmptyCornerEdge);
}

TEST_F(ScoreTest, Edges) {
  set("\
..***o");
  check(2 * Score::Edge);
  set("\
........\
........\
*......*\
*......*\
o......o");
  check(2 * Score::Edge);
  set(5, "\
*......*\
........\
..o***");
  check(4 * Score::Edge);
}

TEST_F(ScoreTest, NonEdgeNextToEmptyCorners) {
  set("\
........\
.*......\
....*o");
  check(Score::EmptyCorner);
  set("\
........\
......*.\
..*o");
  check(Score::EmptyCorner);
  set(5, "\
....*o..\
.*");
  check(Score::EmptyCorner);
  set(5, "\
..*o....\
......*");
  check(Score::EmptyCorner);
}

TEST_F(ScoreTest, NextToEmptyTopEdges) {
  set(1, "\
..*.....\
....*o");
  check(Score::EmptyEdge);
  set(1, "\
..****..\
....*o");
  check(4 * Score::EmptyEdge);
  // not next to empty edge
  set("\
...***..\
....*...\
..*o");
  check(3 * Score::Edge);
  set("\
***.....\
*.......\
*...*o");
  auto score = Score::Corner + 4 * Score::SafeEdge;
  check(score);
  set("\
***.....\
**......\
*...*o");
  check(score + Score::CenterEdge);
  // next to empty
  set("\
****....\
**......\
....*o");
  check(score + Score::EmptyEdge);
  set("\
***.....\
.*......\
*...*o");
  check(Score::Corner + 2 * Score::SafeEdge + Score::Edge + Score::EmptyEdge);
}

TEST_F(ScoreTest, NextToEmptyLeftEdges) {
  set(2, "\
.*..*o");
  check(Score::EmptyEdge);
  set(2, "\
.*..*o..\
.*......\
.*......\
.*");
  check(4 * Score::EmptyEdge);
  set(2, "\
*...*o..\
.*......\
*");
  check(2 * Score::Edge + Score::EmptyEdge);
  set(5, "\
*.......\
.*..*o..\
***");
  check(Score::Corner + 2 * Score::SafeEdge + Score::Edge + Score::EmptyEdge);
}

TEST_F(ScoreTest, NextToEmptyRightEdges) {
  set(2, "\
..*o..*");
  check(Score::EmptyEdge);
  set(2, "\
..*o..*.\
......*.\
......*.\
......*");
  check(4 * Score::EmptyEdge);
  set(2, "\
..*o....\
.......*\
......**");
  check(2 * Score::Edge + Score::EmptyEdge);
  set(5, "\
..*o....\
......**\
.....***");
  check(Score::Corner + 3 * Score::SafeEdge + Score::EmptyEdge);
}

TEST_F(ScoreTest, NextToEmptyBottomEdges) {
  set(5, "\
..*o....\
.....*");
  check(Score::EmptyEdge);
  set(5, "\
..*o....\
..****");
  check(4 * Score::EmptyEdge);
  set(5, "\
..*o....\
....*...\
....**");
  check(2 * Score::Edge + Score::EmptyEdge);
  // no empty edges
  set(5, "\
..*o....\
..****..\
****oooo");
  check(4 * Score::CenterEdge);
}

} // namespace othello
