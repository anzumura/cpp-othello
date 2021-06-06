#include "Score.h"
#include "Board.h"

#include "gtest/gtest.h"
#include <array>
#include <string>
#include <utility>

namespace othello {

using S = FullScore;
using W = WeightedScore;

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
  check(S::BadEdge);
  set("\
......*.\
........\
..*o");
  check(S::BadEdge);
  set("\
........\
*.......\
..*o");
  check(S::BadEdge);
  set("\
........\
.......*\
..*o");
  check(S::BadEdge);
  set("\
.*......\
.......*\
..*o");
  check(2 * S::BadEdge);
}

TEST_F(ScoreTest, EdgeNextToBottomEmptyCorners) {
  set(5, "\
..*o....\
........\
.*");
  check(S::BadEdge);
  set(5, "\
..*o....\
........\
......*");
  check(S::BadEdge);
  set(5, "\
..*o....\
*");
  check(S::BadEdge);
  set(5, "\
..*o....\
.......*");
  check(S::BadEdge);
  set(5, "\
..*o....\
.......*\
......*.");
  check(2 * S::BadEdge);
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
  check(S::BadCenter);
  set("\
........\
......*.\
..*o");
  check(S::BadCenter);
  set(5, "\
....*o..\
.*");
  check(S::BadCenter);
  set(5, "\
..*o....\
......*");
  check(S::BadCenter);
}

TEST_F(ScoreTest, NextToEmptyTopEdges) {
  set(1, "\
..*.....\
....*o");
  check(S::Bad);
  set(1, "\
..****..\
....*o");
  check(4 * S::Bad);
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
  check(score + S::Bad);
  set("\
***.....\
.*......\
*...*o");
  check(S::Corner + 2 * S::SafeEdge + S::Edge + S::Bad);
}

TEST_F(ScoreTest, NextToEmptyLeftEdges) {
  set(2, "\
.*..*o");
  check(S::Bad);
  set(2, "\
.*..*o..\
.*......\
.*......\
.*");
  check(4 * S::Bad);
  set(2, "\
*...*o..\
.*......\
*");
  check(2 * S::Edge + S::Bad);
  set(5, "\
*.......\
.*..*o..\
***");
  check(S::Corner + 2 * S::SafeEdge + S::Edge + S::Bad);
}

TEST_F(ScoreTest, NextToEmptyRightEdges) {
  set(2, "\
..*o..*");
  check(S::Bad);
  set(2, "\
..*o..*.\
......*.\
......*.\
......*");
  check(4 * S::Bad);
  set(2, "\
..*o....\
.......*\
......**");
  check(2 * S::Edge + S::Bad);
  set(5, "\
..*o....\
......**\
.....***");
  check(S::Corner + 3 * S::SafeEdge + S::Bad);
}

TEST_F(ScoreTest, NextToEmptyBottomEdges) {
  set(5, "\
..*o....\
.....*");
  check(S::Bad);
  set(5, "\
..*o....\
..****");
  check(4 * S::Bad);
  set(5, "\
..*o....\
....*...\
....**");
  check(2 * S::Edge + S::Bad);
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
  ASSERT_EQ(board.blackCount(), 28);
  ASSERT_EQ(board.whiteCount(), 24);
  int black = 12 * S::Edge + 4 * S::CenterEdge + 8 * S::Center + S::BadEdge + 3 * S::Bad;
  int white = S::Corner + S::SafeEdge + 4 * S::Edge + 7 * S::CenterEdge + 8 * S::Center + 3 * S::Bad;
  score->score(board, Board::Color::Black, true);
  check(black - white);
  black = 11 * W::Edge + 2 * W::BadEdge + 7 * W::Bad + 4 * W::CenterEdge + 4 * W::Center;
  white = W::Corner + W::BadEdge + W::BadCenter + 4 * W::Edge + 9 * W::Bad + 4 * W::CenterEdge + 4 * W::Center;
  weightedScore->score(board, Board::Color::Black, true);
  checkWeighted(black - white);
}

} // namespace othello
