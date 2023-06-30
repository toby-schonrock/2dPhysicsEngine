cmake_minimum_required(VERSION 3.15)

project(PhysicsEngine)

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

add_library(PhysicsEngine)
target_include_directories(PhysicsEngine INTERFACE /include)