cmake_minimum_required(VERSION 3.0)
project(OneWireHub)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(src)

#set(SOURCE_FILES ./main.cpp)

#add_executable(OneWireHub ${SOURCE_FILES})