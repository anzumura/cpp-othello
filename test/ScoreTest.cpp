#include "Board.h"
#include "Score.h"

#include <array>
#include <string>
#include "gtest/gtest.h"

namespace othello {

class ScoreTest : public ::testing::Test {
 protected:
  void set(int emptyRows, const std::string& initialLayout) { board = Board(initialLayout, emptyRows * Board::RowSize); }
  void set(const std::string& initialLayout) { board = Board(initialLayout); }
  Board board;
  static constexpr std::array firstAndLastRow { 0, 7 };
  static constexpr std::array firstAndLastColumn { "*", ".......*" };
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
  for (int row : firstAndLastRow)
    for (auto col : firstAndLastColumn) {
      set(row, col);
      EXPECT_EQ(Score::score(board, Board::Color::Black), Score::Corner);
      EXPECT_EQ(Score::score(board, Board::Color::White), -Score::Corner);
    }
}

}  // namespace othello