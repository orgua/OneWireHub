cmake_minimum_required(VERSION 3.0)
project(OneWireHub)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(src)
include_directories(./src)

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
        src/DS2890.cpp
        src/OneWireHub.cpp
        src/OneWireItem.cpp
        src/platform.h
        )

add_executable(OneWireHub ${SOURCE_FILES})