add_library(othello_lib Board.cpp Game.cpp Player.cpp Score.cpp)
target_include_directories(othello_lib PUBLIC ../include)
