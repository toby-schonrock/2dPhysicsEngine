cmake_minimum_required(VERSION 3.15)
project(physics_envy)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # makes cmake produce compile_commands.json

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
  set(PROJECT_COMPILE_OPTIONS /Wall /analyze)
  string(APPEND CMAKE_CXX_FLAGS_DEBUG          " /fsanitize=address")
  string(APPEND CMAKE_CXX_FLAGS_RELWITHDEBINFO " /fsanitize=address")
else()
  set(PROJECT_COMPILE_OPTIONS -Wall -Wextra -Wpedantic -Wshadow -Wextra-semi
    -Wmissing-noreturn -Wconversion -Wsign-conversion)
  string(APPEND CMAKE_CXX_FLAGS_DEBUG          " -fsanitize=address,undefined,leak")
  string(APPEND CMAKE_CXX_FLAGS_RELWITHDEBINFO " -fsanitize=address,undefined,leak")
endif()

include(CTest)
add_subdirectory(ext/abseil-cpp)
add_subdirectory(ext/googletest)

# add_library(physics_envy INTERFACE)
# target_include_directories(physics_envy INTERFACE /include)
# target_link_libraries(physics_envy INTERFACE absl::flat_hash_map)

if(BUILD_TESTING)
  add_executable(tests tests/test.cpp)
  target_include_directories(tests PRIVATE include)
  target_link_libraries(tests PRIVATE GTest::gtest_main absl::flat_hash_map)

  include(GoogleTest)
  gtest_discover_tests(tests)
endif()

# if(NOT PROJECT_IS_TOP_LEVEL)
# endif()