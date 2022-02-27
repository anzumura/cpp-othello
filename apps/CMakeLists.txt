add_executable(othello othelloMain.cpp)
target_link_libraries(othello PRIVATE othello_lib)
add_executable(othelloClient OthelloClient.h OthelloClient.cpp
  othelloClientMain.cpp)
