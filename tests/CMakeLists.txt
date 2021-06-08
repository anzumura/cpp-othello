file(GLOB SOURCE_LIST *.cpp)
add_executable(othello_test ${SOURCE_LIST})
target_link_libraries(othello_test PRIVATE othello_lib gtest gmock)
add_test(NAME othello_test COMMAND othello_test)
