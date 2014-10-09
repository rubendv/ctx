cmake_minimum_required (VERSION 2.6)
project (ctxlang)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -ferror-limit=1")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -ferror-limit=1")

include_directories("/opt/local/include")

add_executable(ctxlang main.cpp ast.cpp)
