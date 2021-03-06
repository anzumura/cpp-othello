#include <gtest/gtest.h>

#include <othello/Board.h>

namespace othello {

class BoardTest : public ::testing::Test {
protected:
  void set(size_t emptyRows, const std::string& initialLayout) {
    board = Board(initialLayout, emptyRows * Board::Rows);
  }
  void set(const std::string& initialLayout) { board = Board(initialLayout); }
  void check(size_t emptyRows, const std::string& expectedLayout) const {
    std::string expected(emptyRows * Board::Rows, '.');
    check(expected + expectedLayout);
  }
  void check(const std::string& expected) const {
    EXPECT_EQ(board.toString(),
              expected + std::string(Board::Size - expected.size(), '.'));
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
  ASSERT_TRUE(board.hasValidMoves());
  const auto blackMoves = board.validMoves(Board::Color::Black);
  std::vector<std::string> expectedBlackMoves = {"d3", "c4", "f5", "e6"};
  ASSERT_EQ(blackMoves, expectedBlackMoves);
  const auto whiteMoves = board.validMoves(Board::Color::White);
  std::vector<std::string> expectedWhiteMoves = {"e3", "f4", "c5", "d6"};
  ASSERT_EQ(whiteMoves, expectedWhiteMoves);
  set("\
........\
.******.\
.*oooo*.\
.*o.*o*.\
.*o*.o*.\
.*oooo*.\
.******.\
........");
  auto moves = board.validMoves(Board::Color::White);
  EXPECT_EQ(moves.size(), 30);
}

TEST_F(BoardTest, ValidMovesWithArrays) {
  Board::Boards boards;
  Board::Positions positions;
  const auto result = board.validMoves(Board::Color::Black, boards, positions);
  ASSERT_EQ(result, 4);
  EXPECT_EQ(positions[0], 19);
  EXPECT_EQ(positions[1], 26);
  EXPECT_EQ(positions[2], 37);
  EXPECT_EQ(positions[3], 44);
  board = boards[0];
  check(2, "\
...*....\
...**...\
...*o");
  board = boards[1];
  check(3, "\
..***...\
...*o");
  board = boards[2];
  check(3, "\
...o*...\
...***");
  board = boards[3];
  check(3, "\
...o*...\
...**...\
....*");
}

TEST_F(BoardTest, ToStream) {
  const auto expected = "\
   a b c d e f g h\n\
 +----------------\n\
1| . . . . . . . .\n\
2| . . . . . . . .\n\
3| . . . . . . . .\n\
4| . . . o * . . .\n\
5| . . . * o . . .\n\
6| . . . . . . . .\n\
7| . . . . . . . .\n\
8| . . . . . . . .  Black(*): 2, White(o): 2\n";
  std::stringstream ss;
  ss << board;
  EXPECT_EQ(ss.str(), expected);
}

TEST_F(BoardTest, ToString) {
  check(3, "\
...o*...\
...*o");
}

TEST_F(BoardTest, FlipUp) {
  ASSERT_EQ(board.set("d6", Board::Color::White), 1);
  check(3, "\
...o*...\
...oo...\
...o");
  // test flip up boundary position
  const auto moves = std::array{std::make_pair("o", 1), std::make_pair("*", 0)};
  for (auto& m : moves) {
    set(std::string("*.......") + m.first);
    ASSERT_EQ(board.set("a3", Board::Color::Black), m.second);
  }
}

TEST_F(BoardTest, FlipDown) {
  ASSERT_EQ(board.set("d3", Board::Color::Black), 1);
  check(2, "\
...*....\
...**...\
...*o");
  // test flip down boundary position
  const auto moves = std::array{std::make_pair("o", 1), std::make_pair("*", 0)};
  for (auto& m : moves) {
    set(6, std::string("\
.......*\
.......") + m.first);
    ASSERT_EQ(board.set("h6", Board::Color::White), m.second);
  }
}

TEST_F(BoardTest, FlipLeft) {
  ASSERT_EQ(board.set("f4", Board::Color::White), 1);
  check(3, "\
...ooo..\
...*o");
  // test flip left boundary position
  set("*o");
  ASSERT_EQ(board.set("c1", Board::Color::Black), 1);
  set("oo");
  ASSERT_EQ(board.set("c1", Board::Color::Black), 0);
}

TEST_F(BoardTest, FlipRight) {
  ASSERT_EQ(board.set("c5", Board::Color::White), 1);
  check(3, "\
...o*...\
..ooo");
  // test flip right boundary position
  set(7, "......o*");
  ASSERT_EQ(board.set("f8", Board::Color::Black), 1);
  set(7, "......oo");
  ASSERT_EQ(board.set("f8", Board::Color::Black), 0);
}

TEST_F(BoardTest, FlipUpLeft) {
  ASSERT_EQ(board.set("e6", Board::Color::Black), 1);
  ASSERT_EQ(board.set("f6", Board::Color::White), 1);
  check(3, "\
...o*...\
...*o...\
....*o");
}

TEST_F(BoardTest, FlipUpRight) {
  ASSERT_EQ(board.set("d6", Board::Color::White), 1);
  ASSERT_EQ(board.set("c6", Board::Color::Black), 1);
  check(3, "\
...o*...\
...*o...\
..*o");
}

TEST_F(BoardTest, FlipDownLeft) {
  ASSERT_EQ(board.set("e3", Board::Color::White), 1);
  ASSERT_EQ(board.set("f3", Board::Color::Black), 1);
  check(2, "\
....o*..\
...o*...\
...*o");
}

TEST_F(BoardTest, FlipDownRight) {
  ASSERT_EQ(board.set("d3", Board::Color::Black), 1);
  ASSERT_EQ(board.set("c3", Board::Color::White), 1);
  check(2, "\
..o*....\
...o*...\
...*o");
}

TEST_F(BoardTest, MultipleFlipsDown) {
  for (size_t i = 0; i < Board::RowSub2; ++i) {
    set(i + 1, "\
...***..\
..ooooo");
    std::vector<std::string> moves = {"c", "d", "e", "f", "g"};
    for (auto& m : moves) m += std::to_string(i + 1);
    ASSERT_EQ(board.validMoves(Board::Color::White), moves);
    ASSERT_EQ(board.set(moves[2], Board::Color::White), 3);
    check(i, "\
....o...\
...ooo..\
..ooooo");
  }
}

TEST_F(BoardTest, MultipleFlipsUp) {
  for (size_t i = 0; i < Board::RowSub2; ++i) {
    set(i, "\
..*****.\
...ooo");
    std::vector<std::string> moves = {"c", "d", "e", "f", "g"};
    for (auto& m : moves) m += std::to_string(i + 3);
    ASSERT_EQ(board.validMoves(Board::Color::Black), moves);
    ASSERT_EQ(board.set(moves[2], Board::Color::Black), 3);
    check(i, "\
..*****.\
...***..\
....*");
  }
}

TEST_F(BoardTest, MultipleFlipsLeft) {
  for (size_t i = 0; i < Board::RowSub2; ++i) {
    const auto f = [i](const char* s) {
      std::string result(i, '.');
      result.append(s);
      return result + std::string(Board::RowSub2 - 1 - i, '.');
    };
    set(f("o..") + f("o*.") + f("o*.") + f("o*.") + f("o"));
    std::vector<std::string> moves;
    for (size_t j = 1; j < Board::RowSub2; ++j)
      moves.emplace_back(std::string(1, 'c' + static_cast<char>(i)) +
                         std::to_string(j));
    ASSERT_EQ(board.validMoves(Board::Color::White), moves);
    ASSERT_EQ(board.set(moves[2], Board::Color::White), 3);
    check(f("o..") + f("oo.") + f("ooo") + f("oo.") + f("o"));
  }
}

TEST_F(BoardTest, MultipleFlipsRight) {
  for (size_t i = 0; i < Board::RowSub2; ++i) {
    const auto f = [i](const char* s) {
      std::string result(i, '.');
      result.append(s);
      return result + std::string(Board::RowSub2 - 1 - i, '.');
    };
    set(f("..*") + f(".o*") + f(".o*") + f(".o*") + f("..*"));
    std::vector<std::string> moves;
    for (auto j = 1; j < Board::RowSub2; ++j)
      moves.emplace_back(std::string(1, 'a' + static_cast<char>(i)) +
                         std::to_string(j));
    ASSERT_EQ(board.validMoves(Board::Color::Black), moves);
    ASSERT_EQ(board.set(moves[2], Board::Color::Black), 3);
    check(f("..*") + f(".**") + f("***") + f(".**") + f("..*"));
  }
}

TEST_F(BoardTest, FlipHittingRightEdge) {
  set(1, "\
......oo\
*ooooo.o\
......o.\
......*");
  ASSERT_EQ(board.set("g3", Board::Color::Black), 6);
  check(1, "\
......oo\
*******o\
......*.\
......*");
}

TEST_F(BoardTest, FlipHittingBottomEdge) {
  set("\
..o.....\
..*.....\
..*.....\
..*.....\
..*.....\
..*.....\
.*.****o\
.**.....");
  ASSERT_EQ(board.set("c7", Board::Color::White), 9);
  check("\
..o.....\
..o.....\
..o.....\
..o.....\
..o.....\
..o.....\
.*oooooo\
.**.....");
}

TEST_F(BoardTest, FlipHittingLeftEdge) {
  set("\
*......*\
.o.....o\
..o....o\
...o...o\
....o..o\
.....o.o\
......oo\
ooooooo.");
  ASSERT_EQ(board.set("h8", Board::Color::Black), 12);
  check("\
*......*\
.*.....*\
..*....*\
...*...*\
....*..*\
.....*.*\
......**\
ooooooo*");
}

TEST_F(BoardTest, FlipHittingTopEdge) {
  set("\
..o..*..\
*.o.o...\
.ooo....\
*o.oooo*\
..oo....\
..o.o...\
..o..o..\
..*...*.");
  ASSERT_EQ(board.set("c4", Board::Color::Black), 14);
  set("\
..o..*..\
*.o.*...\
.*o*....\
********\
..**....\
..*.*...\
..*..*..\
..*...*.");
}

TEST_F(BoardTest, SetFailsForBadRowOrColumn) {
  for (auto v : Board::Colors) {
    // bad lengths
    EXPECT_EQ(board.set("", v), Board::BadSize);
    EXPECT_EQ(board.set("f", v), Board::BadSize);
    EXPECT_EQ(board.set("f44", v), Board::BadSize);
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

} // namespace othello
