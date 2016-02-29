cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
        DS18B20.cpp
        DS2401.cpp
        DS2405.cpp
        DS2408.cpp
        DS2413.cpp
        DS2423.cpp
        DS2433.cpp
        DS2438.cpp
        DS2450.cpp
        DS2890.cpp
        OneWireHub.cpp
        OneWireItem.cpp
        platform.h
        )

add_executable(OneWireSRC ${SOURCE_FILES})