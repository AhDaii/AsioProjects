cmake_minimum_required(VERSION 3.25)
project(protobuf_test)

set(CMAKE_CXX_STANDARD 20)
list(APPEND CMAKE_SYSTEM_PREFIX_PATH "/usr/local/protobuf/include")
include_directories(/usr/local/protobuf/include)

# g++ -o test main.cpp msg.pb.cc -lprotobuf -I /usr/local/protobuf/include -L /usr/local/protobuf/lib -std=c++11

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

link_directories(/usr/local/protobuf/lib)


add_executable(protobuf_test main.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(protobuf_test protobuf)
