add_executable(othello othello.cpp)
target_link_libraries(othello PRIVATE othello_lib)
add_executable(othelloClient othelloClient.cpp)
