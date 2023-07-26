cmake_minimum_required(VERSION 3.25)
project(SyncClient_JsonCPP_v2)

set(CMAKE_CXX_STANDARD 20)

set(BOOST_ROOT /opt/homebrew/Cellar/boost/1.82.0_1)
set(BOOST_INCLUDEDIR /opt/homebrew/Cellar/boost/1.82.0_1/include)
set(BOOST_LIBRARY_DIR /opt/homebrew/Cellar/boost/1.82.0_1/lib)

find_package(
  Boost 1.82.0
  COMPONENTS system filesystem
  REQUIRED)

include_directories(${Boost_INCLUDE_DIR})

if(NOT Boost_FOUND)
  message(FATAL_ERROR "Could not find boost!")
endif()

include_directories(/opt/homebrew/Cellar/jsoncpp/1.9.5/include)
add_library(jsoncpp SHARED IMPORTED)
set_target_properties(
  jsoncpp
  PROPERTIES IMPORTED_LOCATION
             "/opt/homebrew/Cellar/jsoncpp/1.9.5/lib/libjsoncpp.25.dylib"
             INTERFACE_INCLUDE_DIRECTORIES
             "/opt/homebrew/Cellar/jsoncpp/1.9.5/include/json")
add_executable(SyncClient_JsonCPP_v2 main.cpp)
target_link_libraries(SyncClient_JsonCPP_v2 jsoncpp)
