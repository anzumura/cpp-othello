#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <othello/Player.h>

namespace othello {

using ::testing::_;
using ::testing::Return;

using C = Board::Color;
using Set = const Board::Set&;

class MockScore : public Score {
public:
  MOCK_METHOD(std::string, toString, (), (const, override));
  MOCK_METHOD(int, scoreBoard, (const Board&, Set, Set, bool),
              (const, override));
  MOCK_METHOD(int, scoreCell, (int, int, int, Set, Set, Set),
              (const, override));
};

class PlayerTest : public ::testing::Test {
protected:
  void scoreChildren(const Board& b, C c, int scoreStart, int jump = 1) {
    Board::Boards boards;
    const auto moves = b.validMoves(c, boards);
    // use WillRepeatedly instead of WillOnce because some child nodes may not
    // be scored due to alpha-beta pruning
    for (auto i = 0, j = scoreStart; i < moves; ++i, j += jump)
      EXPECT_CALL(*score, scoreBoard(boards[i], _, _, _))
        .WillRepeatedly(Return(j));
  }

  // 4 initial valid moves for Black
  Board b1 = Board(3, "\
..***...\
...*o");
  Board b2 = Board(3, "\
...o*...\
...***");
  Board b3 = Board(3, "\
...o*...\
...**...\
....*");
  Board b4 = Board(2, "\
...*....\
...**...\
...*o");

  Board board;
  std::shared_ptr<MockScore> score = std::make_shared<MockScore>();
  std::unique_ptr<Player> playerDepth1 =
    std::make_unique<ComputerPlayer>(C::Black, 1, false, score);
  std::unique_ptr<Player> playerDepth2 =
    std::make_unique<ComputerPlayer>(C::Black, 2, false, score);
};

TEST_F(PlayerTest, MoveDepth1) {
  // make 'b3' have the highest score
  EXPECT_CALL(*score, scoreBoard(b1, _, _, _)).WillOnce(Return(10));
  EXPECT_CALL(*score, scoreBoard(b2, _, _, _)).WillOnce(Return(7));
  EXPECT_CALL(*score, scoreBoard(b3, _, _, _)).WillOnce(Return(12));
  EXPECT_CALL(*score, scoreBoard(b4, _, _, _)).WillOnce(Return(-5));
  playerDepth1->move(board, true, {});
  EXPECT_EQ(board, b3);
}

TEST_F(PlayerTest, MultipleMovesDepth1) {
  // make 'b1' and 'b4' have the same high score
  EXPECT_CALL(*score, scoreBoard(b1, _, _, _))
    .Times(2)
    .WillRepeatedly(Return(22));
  EXPECT_CALL(*score, scoreBoard(b2, _, _, _)).WillOnce(Return(7));
  EXPECT_CALL(*score, scoreBoard(b3, _, _, _)).WillOnce(Return(12));
  EXPECT_CALL(*score, scoreBoard(b4, _, _, _))
    .Times(2)
    .WillRepeatedly(Return(22));
  playerDepth1->move(board, true, {});
  // with random 'false' the result will be 'b4' since that valid move is found
  // first, i.e. the location is 'd3' which is found first when calculating
  // valid moves (starting at top left)
  EXPECT_EQ(board, b4);
}

TEST_F(PlayerTest, MoveDepth2) {
  // set scores for depth 2 nodes
  // b1 children will have scores of 10, 16, ...
  // b2 children will have scores of 11, 12, ...
  scoreChildren(b1, C::White, 10, 6);
  scoreChildren(b2, C::White, 11);
  scoreChildren(b3, C::White, -20);
  scoreChildren(b4, C::White, 0);
  playerDepth2->move(board, true, {});
  // result should be b2 because depth 2 is a 'minimizing' level so 'b1' will
  // get a value of 10 (even though it has children with much higher scores)
  // whereas 'b2' will get a value of 11 (the min from that level) and then 11 >
  // 10 for 'maximizing' level 1
  EXPECT_EQ(board, b2);
}

} // namespace othello
