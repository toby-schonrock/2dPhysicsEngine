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



if(PROJECT_IS_TOP_LEVEL)
  add_executable(physics_envy main.cpp)
  target_include_directories(physics_envy PRIVATE /include)
else()
  add_library(physics-envy INTERFACE)
  target_include_directories(physics_envy INTERFACE /include)
endif()
