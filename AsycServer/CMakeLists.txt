cmake_minimum_required(VERSION 3.25)
project(AsycServer)

set(CMAKE_CXX_STANDARD 20)

set(BOOST_ROOT /opt/homebrew/Cellar/boost/1.82.0_1)
set(BOOST_INCLUDEDIR /opt/homebrew/Cellar/boost/1.82.0_1/include)
set(BOOST_LIBRARY_DIR /opt/homebrew/Cellar/boost/1.82.0_1/lib)

find_package(Boost 1.82.0 COMPONENTS system filesystem REQUIRED)

include_directories(${Boost_INCLUDE_DIR})

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()

aux_source_directory(src _SOURCES)

include_directories(include)

add_executable(AsycServer main.cpp ${_SOURCES})
