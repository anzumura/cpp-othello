cmake_minimum_required(VERSION 3.14..3.22)
project(othello VERSION 1.0
                DESCRIPTION "Othello Game"
                LANGUAGES CXX)

### Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
  message(FATAL_ERROR "can't build in a source directory")
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_CXX_STANDARD 20)
  # enable some warnings and turn them into errors (-W is the same as -Wextra)
  add_compile_options(-Werror -W -Wall -Wunused -Wconversion -Wshadow)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # '-Wdeprecated' causes an error with GCC on line 918 in 'gtest-internal.h'
    add_compile_options(-Wdeprecated -Wunreachable-code-aggressive
      -Wno-logical-op-parentheses)
    set(SANITIZER_FLAGS "-fsanitize=address -fno-omit-frame-pointer -O1 \
-fno-optimize-sibling-calls")
    # need to set env for stack: ASAN_OPTIONS=detect_stack_use_after_return=1
    # string(APPEND SANITIZER_FLAGS "-fsanitize-address-use-after-scope")
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " ${SANITIZER_FLAGS}")
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    add_compile_options(-Wno-parentheses)
  endif()
  include(CTest)
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  # 1.11.0 was released on June 11, 2021 (or change to HEAD)
  GIT_TAG release-1.11.0
)
FetchContent_MakeAvailable(googletest)

# On macOS boost can be installed using 'homebrew install boost' which pulls in
# version 1.78
find_package(Boost 1.78.0 REQUIRED COMPONENTS system)
if(Boost_FOUND)
  # add 'SYSTEM' to skip warnings ('conversion', 'deprecated', 'shadow', etc.)
  include_directories(SYSTEM ${Boost_INCLUDE_DIR})
  message("-- Boost found: ${Boost_VERSION_STRING}")
else()
  message(FATAL_ERROR "Boost not found!")
endif()

add_subdirectory(src)
add_subdirectory(apps)

if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING)
  AND BUILD_TESTING)
  add_subdirectory(tests)
endif()
