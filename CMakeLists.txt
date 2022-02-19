cmake_minimum_required(VERSION 3.14..3.22)
project(othello VERSION 1.0
                DESCRIPTION "Othello Game"
                LANGUAGES CXX)

### Require out-of-source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if(EXISTS "${LOC_PATH}")
  message(FATAL_ERROR "You cannot build in a source directory (or any directory with a CMakeLists.txt file).")
endif()

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_EXTENSIONS OFF)
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # enable some warnings and turn them into errors (skip -Wdeprecated for now because of boost)
    set(WARN_FLAGS "-Werror -Wall -Wextra -Wunused -Wc++20-compat-pedantic -Wunreachable-code-aggressive")
    # disable a warning
    set(WARN_FLAGS "${WARN_FLAGS} -Wno-logical-op-parentheses")
    set(SANITIZER_FLAGS "-fsanitize=address -fno-omit-frame-pointer -O1 -fno-optimize-sibling-calls")
    # need to set env for after stack reporting: ASAN_OPTIONS=detect_stack_use_after_return=1
    # set(SANITIZER_STACK_FLAGS "-fsanitize-address-use-after-scope")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${SANITIZER_FLAGS} ${SANITIZER_STACK_FLAGS} ${WARN_FLAGS}")
  endif()
  include(CTest)
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.11.0
)
FetchContent_MakeAvailable(googletest)

# On macOS boost can be installed using 'homebrew install boost' which pulls in version 1.78
find_package(Boost 1.78.0 REQUIRED COMPONENTS system)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIR})
  message("-- Boost found: ${Boost_VERSION_STRING}")
else()
  message(FATAL_ERROR "Boost not found!")
endif()

add_subdirectory(src)
add_subdirectory(apps)

if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING) AND BUILD_TESTING)
  add_subdirectory(tests)
endif()
