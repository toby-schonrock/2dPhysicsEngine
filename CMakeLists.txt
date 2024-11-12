cmake_minimum_required(VERSION 3.28)
project(physics_envy)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # makes cmake produce compile_commands.json
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

if(PROJECT_IS_TOP_LEVEL)
  if(MSVC)
    set(PROJECT_COMPILE_OPTIONS /Wall /analyze)
    string(APPEND CMAKE_CXX_FLAGS_DEBUG " /fsanitize=address")
    string(APPEND CMAKE_CXX_FLAGS_RELWITHDEBINFO " /fsanitize=address")
  else()
    set(PROJECT_COMPILE_OPTIONS -Wall -Wextra -Wpedantic -Wshadow -Wextra-semi
      # -ftime-trace # for compile time reporting
      -Wmissing-noreturn -Wconversion -Wsign-conversion -Wno-ignored-attributes)
    if(CMAKE_BUILD_TYPE STREQUAL "debug")
      if(NOT MINGW) # sanitizers are not working under mingw
        if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
          message(WARNING "FreeBSD detected: skipping santizers which cause weird errors in OpenSSL")
        else()
          string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=address,undefined,leak")
          # string(APPEND CMAKE_CXX_FLAGS_DEBUG " -fsanitize=thread") # alternative to above
        endif()
      endif()
    endif()
  endif()
endif()

add_subdirectory(ext/abseil-cpp)

add_library(physics_envy INTERFACE)
target_include_directories(physics_envy INTERFACE ./include)
target_link_libraries(physics_envy INTERFACE absl::flat_hash_map)
target_precompile_headers(physics_envy INTERFACE [["absl/container/flat_hash_map.h"]]) # prevents changes to physics_envy requiring abseil to be recompiled

add_subdirectory(tests)
