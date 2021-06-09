file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${othello_SOURCE_DIR}/include/othello/*.h")
file(GLOB SOURCE_LIST *.cpp)
add_library(othello_lib ${SOURCE_LIST} ${HEADER_LIST})
target_include_directories(othello_lib PUBLIC ../include)
