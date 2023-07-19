cmake_minimum_required(VERSION 3.25)
project(SyncClient_Protobuf)


set(CMAKE_CXX_STANDARD 20)
set(BOOST_ROOT /opt/homebrew/Cellar/boost/1.82.0_1)
set(BOOST_INCLUDEDIR /opt/homebrew/Cellar/boost/1.82.0_1/include)
set(BOOST_LIBRARY_DIR /opt/homebrew/Cellar/boost/1.82.0_1/lib)

find_package(Boost 1.82.0 COMPONENTS system filesystem REQUIRED)

include_directories(${Boost_INCLUDE_DIR})

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Could not find boost!")
endif()

############PROTOBUF###########
list(APPEND CMAKE_SYSTEM_PREFIX_PATH "/usr/local/protobuf/include")
include_directories(/usr/local/protobuf/include)
find_package(Protobuf REQUIRED)
if (PROTOBUF_FOUND)
    message("protobuf found")
else ()
    message(FATAL_ERROR "Cannot find Protobuf")
endif ()
file(GLOB NART_PROTOS *.proto)
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${NART_PROTOS})
message("PROTO_SRCS = ${PROTO_SRCS}")
message("PROTO_HDRS = ${PROTO_HDRS}")

############PROTOBUF###########

add_executable(SyncClient_Protobuf main.cpp MsgData.pb.cc)
target_link_libraries(SyncClient_Protobuf protobuf)