#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "Formula.h"

using ::testing::StartsWith;

TEST(blaTest, test1) {
  //arrange
  //act
  //assert
  EXPECT_EQ(Formula::bla(0),  0);
  EXPECT_EQ(Formula::bla(10), 20);
  EXPECT_EQ(Formula::bla(50), 100);
}

TEST(blaTest, test2) {
  EXPECT_THAT(Formula::str(), StartsWith("he"));
}
