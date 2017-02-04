## \author Hans J. Johnson
## In directory above this directory
## mkdir -p hw1-bld
## cd hw1-bld
## cmake -G "CodeBlocks - Unix Makefiles" ../homework1
## --- or ---
## cmake -G "Xcode" ../homework1
## --- or ---
## cmake -G "Unix Makefile" ../homework1
## -- or ---
## Use cmake GUI on windows and select your favorite development environment (i.e. MSVS 2015)
## -- or ---
## cmake --help :  Read documentation for more options
##
## finally:  Build you program
cmake_minimum_required(VERSION 3.0.2)
set(CMAKE_CXX_STANDARD 14)

project(plot CXX)

##==============================
## This section sets up default compiler options for different build environments
#http://stackoverflow.com/questions/10046114/in-cmake-how-can-i-test-if-the-compiler-is-clang
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang") #Both AppleClang and Linux Clang
  # using Clang
  set(CMAKE_CXX_FLAGS "-std=c++14 -g -Wall -Wno-missing-braces -Wcast-align -Wdisabled-optimization -Wextra -Wformat=2 -Winvalid-pch -Wno-format-nonliteral -Wpointer-arith -Wshadow -Wunused -Wwrite-strings -funit-at-a-time -Wno-strict-overflow -Wno-deprecated -Wno-invalid-offsetof -Woverloaded-virtual")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
  set(CMAKE_CXX_FLAGS "-std=c++14 -g -Wall -Wno-missing-braces -Wcast-align -Wdisabled-optimization -Wextra -Wformat=2 -Winvalid-pch -Wno-format-nonliteral -Wpointer-arith -Wshadow -Wunused -Wwrite-strings -funit-at-a-time -Wno-strict-overflow -Wno-deprecated -Wno-invalid-offsetof -Woverloaded-virtual")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
  set(CMAKE_CXX_FLAGS "-std=c++14 -g -Wall")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  ## See https://msdn.microsoft.com/en-us/library/hh567368.aspx#cpp14table for details
  ## TODO: ONLY MSVS 2015 provides sufficient c++14 support for this course
  ## if(CMAKE_CXX_COMPILER_VERSION VERSIONLESS

  # using Visual Studio C++, Force to always compile with at least /W2, replacing all other lower values
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-2]")
    string(REGEX REPLACE "/W[0-2]" "/W2" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W2")
  endif()
endif()

include_directories(${CMAKE_CURRENT_LIST_DIR})
add_subdirectory(examples)
