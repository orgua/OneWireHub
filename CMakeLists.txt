cmake_minimum_required(VERSION 3.0)
project(OneWireHub)

add_definitions(-std=c++11 -Wall -Wextra -pipe -march=native -mtune=native)

#add_subdirectory(src)
#include_directories(./src)

set(SOURCE_FILES
        main.cpp
        src/BAE910.cpp
        src/DS18B20.cpp
        src/DS2401.cpp
        src/DS2405.cpp
        src/DS2408.cpp
        src/DS2413.cpp
        src/DS2423.cpp
        src/DS2431.cpp
        src/DS2433.cpp
        src/DS2438.cpp
        src/DS2450.cpp
        src/DS2502.cpp
        src/DS2506.cpp
        src/DS2890.cpp
        src/OneWireHub.cpp
        src/OneWireHub_config.h
        src/OneWireItem.cpp
        src/platform.cpp
        )

add_executable(OneWireHub ${SOURCE_FILES})