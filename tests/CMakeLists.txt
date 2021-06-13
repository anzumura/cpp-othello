add_executable(othello_test BoardTest.cpp PlayerTest.cpp ScoreTest.cpp testMain.cpp)
target_link_libraries(othello_test PRIVATE othello_lib gtest gmock)
add_test(NAME othello_test COMMAND othello_test)
