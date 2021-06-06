#include "Board.h"
#include "Score.h"

#include "gtest/gtest.h"
#include <array>
#include <string>
#include <utility>

namespace othello {

using S = FullScore;

class ScoreTest : public ::testing::Test {
protected:
  void set(int emptyRows, const std::string& initialLayout) { board = Board(initialLayout, emptyRows * Board::Rows); }
  void set(const std::string& initialLayout) { board = Board(initialLayout); }
  void check(int s) {
    for (auto c : Board::Colors)
      ASSERT_EQ(score->score(board, c), c == Board::Color::Black ? s : -s) << "FullScore for: " << c;
  }
  void checkWeighted(int s) {
    for (auto c : Board::Colors)
      ASSERT_EQ(weightedScore->score(board, c), c == Board::Color::Black ? s : -s) << "WeightedScore for: " << c;
  }
  Board board;
  std::unique_ptr<Score> score = std::move(std::make_unique<FullScore>());
  std::unique_ptr<Score> weightedScore = std::move(std::make_unique<WeightedScore>());
};

TEST_F(ScoreTest, InitialPosition) {
  check(0);
  checkWeighted(0);
}

TEST_F(ScoreTest, Win) {
  set("*");
  check(Score::Win);
  checkWeighted(Score::Win);
  set(std::string(64, '*'));
  check(Score::Win);
  checkWeighted(Score::Win);
}

TEST_F(ScoreTest, Draw) {
  set(""); // board with no valid moves and equal counts
  check(0);
  checkWeighted(0);
  set("*.o");
  check(0);
  checkWeighted(0);
}

TEST_F(ScoreTest, AfterOneFlip) {
  board.set("d3", Board::Color::Black); // flips 1 resulting in 4 black and 1 white
  check(3);
  // for WeightedScore, black now has 3 (of the initial 4 pieces) and white has 1 - each of these is worth 1
  // and the new location 'd3' is worth 0 for a total score of 2 for black
  checkWeighted(2);
}

TEST_F(ScoreTest, Corners) {
  // use boards with at least one valid move in order to avoid 'Win' scores
  std::array corners{std::make_pair(0, "\
*.......\
........\
...*o"),
                     std::make_pair(0, "\
.......*\
........\
...*o"),
                     std::make_pair(5, "\
...*o...\
........\
*"),
                     std::make_pair(5, "\
...*o...\
........\
.......*")};
  for (auto c : corners) {
    set(c.first, c.second);
    check(S::Corner);
    checkWeighted(4);
  }
}

TEST_F(ScoreTest, SafeHorizontalEdge) {
  set("\
**......\
........\
..*o");
  check(S::Corner + S::SafeEdge);
  set("\
**....**\
........\
..*o");
  check(2 * (S::Corner + S::SafeEdge));
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
  set(5, "\
....o*..\
........\
**o");
  check(S::Corner + S::SafeEdge - S::Edge);
}

TEST_F(ScoreTest, SafeVerticalEdge) {
  set("\
.......*\
.......*\
..*o");
  check(S::Corner + S::SafeEdge);
  set("\
.......*\
.......*\
........\
........\
..*o....\
........\
.......*\
.......*");
  check(2 * (S::Corner + S::SafeEdge));
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
  check(S::EmptyCornerEdge);
  set("\
......*.\
........\
..*o");
  check(S::EmptyCornerEdge);
  set("\
........\
*.......\
..*o");
  check(S::EmptyCornerEdge);
  set("\
........\
.......*\
..*o");
  check(S::EmptyCornerEdge);
  set("\
.*......\
.......*\
..*o");
  check(2 * S::EmptyCornerEdge);
}

TEST_F(ScoreTest, EdgeNextToBottomEmptyCorners) {
  set(5, "\
..*o....\
........\
.*");
  check(S::EmptyCornerEdge);
  set(5, "\
..*o....\
........\
......*");
  check(S::EmptyCornerEdge);
  set(5, "\
..*o....\
*");
  check(S::EmptyCornerEdge);
  set(5, "\
..*o....\
.......*");
  check(S::EmptyCornerEdge);
  set(5, "\
..*o....\
.......*\
......*.");
  check(2 * S::EmptyCornerEdge);
}

TEST_F(ScoreTest, Edges) {
  set("\
..***o");
  check(2 * S::Edge);
  set("\
........\
........\
*......*\
*......*\
o......o");
  check(2 * S::Edge);
  set(5, "\
*......*\
........\
..o***");
  check(4 * S::Edge);
}

TEST_F(ScoreTest, NonEdgeNextToEmptyCorners) {
  set("\
........\
.*......\
....*o");
  check(S::EmptyCorner);
  set("\
........\
......*.\
..*o");
  check(S::EmptyCorner);
  set(5, "\
....*o..\
.*");
  check(S::EmptyCorner);
  set(5, "\
..*o....\
......*");
  check(S::EmptyCorner);
}

TEST_F(ScoreTest, NextToEmptyTopEdges) {
  set(1, "\
..*.....\
....*o");
  check(S::EmptyEdge);
  set(1, "\
..****..\
....*o");
  check(4 * S::EmptyEdge);
  // not next to empty edge
  set("\
...***..\
....*...\
..*o");
  check(3 * S::Edge);
  set("\
***.....\
*.......\
*...*o");
  auto score = S::Corner + 4 * S::SafeEdge;
  check(score);
  set("\
***.....\
**......\
*...*o");
  check(score + S::CenterEdge);
  // next to empty
  set("\
****....\
**......\
....*o");
  check(score + S::EmptyEdge);
  set("\
***.....\
.*......\
*...*o");
  check(S::Corner + 2 * S::SafeEdge + S::Edge + S::EmptyEdge);
}

TEST_F(ScoreTest, NextToEmptyLeftEdges) {
  set(2, "\
.*..*o");
  check(S::EmptyEdge);
  set(2, "\
.*..*o..\
.*......\
.*......\
.*");
  check(4 * S::EmptyEdge);
  set(2, "\
*...*o..\
.*......\
*");
  check(2 * S::Edge + S::EmptyEdge);
  set(5, "\
*.......\
.*..*o..\
***");
  check(S::Corner + 2 * S::SafeEdge + S::Edge + S::EmptyEdge);
}

TEST_F(ScoreTest, NextToEmptyRightEdges) {
  set(2, "\
..*o..*");
  check(S::EmptyEdge);
  set(2, "\
..*o..*.\
......*.\
......*.\
......*");
  check(4 * S::EmptyEdge);
  set(2, "\
..*o....\
.......*\
......**");
  check(2 * S::Edge + S::EmptyEdge);
  set(5, "\
..*o....\
......**\
.....***");
  check(S::Corner + 3 * S::SafeEdge + S::EmptyEdge);
}

TEST_F(ScoreTest, NextToEmptyBottomEdges) {
  set(5, "\
..*o....\
.....*");
  check(S::EmptyEdge);
  set(5, "\
..*o....\
..****");
  check(4 * S::EmptyEdge);
  set(5, "\
..*o....\
....*...\
....**");
  check(2 * S::Edge + S::EmptyEdge);
  // no empty edges
  set(5, "\
..*o....\
..****..\
****oooo");
  check(4 * S::CenterEdge);
}

TEST_F(ScoreTest, ComplexBoard) {
  set("\
..***...\
..ooo*..\
oooo****\
ooo*o***\
oo*o****\
ooooo***\
*oo*o*..\
oo*****.");
  int black = 12 * S::Edge + 4 * S::CenterEdge + 8 * S::Center + S::EmptyCornerEdge + 3 * S::EmptyEdge;
  int white = S::Corner + S::SafeEdge + 4 * S::Edge + 7 * S::CenterEdge + 8 * S::Center + 3 * S::EmptyEdge;
  score->score(board, Board::Color::Black, true);
  check(black - white);
}

} // namespace othello
