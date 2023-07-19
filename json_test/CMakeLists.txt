cmake_minimum_required(VERSION 3.25)
project(json_test)

set(CMAKE_CXX_STANDARD 20)
include_directories(/opt/homebrew/Cellar/jsoncpp/1.9.5/include)
add_library(jsoncpp SHARED IMPORTED)
set_target_properties(jsoncpp PROPERTIES
        IMPORTED_LOCATION "/opt/homebrew/Cellar/jsoncpp/1.9.5/lib/libjsoncpp.25.dylib"
        INTERFACE_INCLUDE_DIRECTORIES "/opt/homebrew/Cellar/jsoncpp/1.9.5/include/json"
        )
add_executable(json_test main.cpp)
target_link_libraries(json_test jsoncpp)