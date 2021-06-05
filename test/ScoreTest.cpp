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
      ASSERT_EQ(score->score(board, c), c == Board::Color::Black ? s : -s) << "Color: " << c;
  }
  Board board;
  std::unique_ptr<Score> score = std::move(std::make_unique<FullScore>());
};

TEST_F(ScoreTest, InitialPosition) {
  check(0);
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
    check(FullScore::Corner);
  }
}

TEST_F(ScoreTest, SafeHorizontalEdge) {
  set("\
**......\
........\
..*o");
  check(FullScore::Corner + FullScore::SafeEdge);
  set("\
**....**\
........\
..*o");
  check(2 * (FullScore::Corner + FullScore::SafeEdge));
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
  check(FullScore::Corner + FullScore::SafeEdge);
  set("\
.......*\
.......*\
........\
........\
..*o....\
........\
.......*\
.......*");
  check(2 * (FullScore::Corner + FullScore::SafeEdge));
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
  check(FullScore::EmptyCornerEdge);
  set("\
......*.\
........\
..*o");
  check(FullScore::EmptyCornerEdge);
  set("\
........\
*.......\
..*o");
  check(FullScore::EmptyCornerEdge);
  set("\
........\
.......*\
..*o");
  check(FullScore::EmptyCornerEdge);
  set("\
.*......\
.......*\
..*o");
  check(2 * FullScore::EmptyCornerEdge);
}

TEST_F(ScoreTest, EdgeNextToBottomEmptyCorners) {
  set(5, "\
..*o....\
........\
.*");
  check(FullScore::EmptyCornerEdge);
  set(5, "\
..*o....\
........\
......*");
  check(FullScore::EmptyCornerEdge);
  set(5, "\
..*o....\
*");
  check(FullScore::EmptyCornerEdge);
  set(5, "\
..*o....\
.......*");
  check(FullScore::EmptyCornerEdge);
  set(5, "\
..*o....\
.......*\
......*.");
  check(2 * FullScore::EmptyCornerEdge);
}

TEST_F(ScoreTest, Edges) {
  set("\
..***o");
  check(2 * FullScore::Edge);
  set("\
........\
........\
*......*\
*......*\
o......o");
  check(2 * FullScore::Edge);
  set(5, "\
*......*\
........\
..o***");
  check(4 * FullScore::Edge);
}

TEST_F(ScoreTest, NonEdgeNextToEmptyCorners) {
  set("\
........\
.*......\
....*o");
  check(FullScore::EmptyCorner);
  set("\
........\
......*.\
..*o");
  check(FullScore::EmptyCorner);
  set(5, "\
....*o..\
.*");
  check(FullScore::EmptyCorner);
  set(5, "\
..*o....\
......*");
  check(FullScore::EmptyCorner);
}

TEST_F(ScoreTest, NextToEmptyTopEdges) {
  set(1, "\
..*.....\
....*o");
  check(FullScore::EmptyEdge);
  set(1, "\
..****..\
....*o");
  check(4 * FullScore::EmptyEdge);
  // not next to empty edge
  set("\
...***..\
....*...\
..*o");
  check(3 * FullScore::Edge);
  set("\
***.....\
*.......\
*...*o");
  auto score = FullScore::Corner + 4 * FullScore::SafeEdge;
  check(score);
  set("\
***.....\
**......\
*...*o");
  check(score + FullScore::CenterEdge);
  // next to empty
  set("\
****....\
**......\
....*o");
  check(score + FullScore::EmptyEdge);
  set("\
***.....\
.*......\
*...*o");
  check(FullScore::Corner + 2 * FullScore::SafeEdge + FullScore::Edge + FullScore::EmptyEdge);
}

TEST_F(ScoreTest, NextToEmptyLeftEdges) {
  set(2, "\
.*..*o");
  check(FullScore::EmptyEdge);
  set(2, "\
.*..*o..\
.*......\
.*......\
.*");
  check(4 * FullScore::EmptyEdge);
  set(2, "\
*...*o..\
.*......\
*");
  check(2 * FullScore::Edge + FullScore::EmptyEdge);
  set(5, "\
*.......\
.*..*o..\
***");
  check(FullScore::Corner + 2 * FullScore::SafeEdge + FullScore::Edge + FullScore::EmptyEdge);
}

TEST_F(ScoreTest, NextToEmptyRightEdges) {
  set(2, "\
..*o..*");
  check(FullScore::EmptyEdge);
  set(2, "\
..*o..*.\
......*.\
......*.\
......*");
  check(4 * FullScore::EmptyEdge);
  set(2, "\
..*o....\
.......*\
......**");
  check(2 * FullScore::Edge + FullScore::EmptyEdge);
  set(5, "\
..*o....\
......**\
.....***");
  check(FullScore::Corner + 3 * FullScore::SafeEdge + FullScore::EmptyEdge);
}

TEST_F(ScoreTest, NextToEmptyBottomEdges) {
  set(5, "\
..*o....\
.....*");
  check(FullScore::EmptyEdge);
  set(5, "\
..*o....\
..****");
  check(4 * FullScore::EmptyEdge);
  set(5, "\
..*o....\
....*...\
....**");
  check(2 * FullScore::Edge + FullScore::EmptyEdge);
  // no empty edges
  set(5, "\
..*o....\
..****..\
****oooo");
  check(4 * FullScore::CenterEdge);
}

} // namespace othello
