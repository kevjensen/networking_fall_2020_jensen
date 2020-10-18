cmake_minimum_required(VERSION 3.8)
project(in_class_udp_example)

set(CMAKE_CXX_STANDARD 11)

set(CLIENT_SOURCE in_class_udp_client.cpp udp_utils.cpp tictactoe.cpp)
set(SERVER_SOURCE in_class_udp_server.cpp udp_utils.cpp)

set(CLIENT_STRUCT_SOURCE in_class_udp_client_struct.cpp udp_utils.cpp)
set(SERVER_STRUCT_SOURCE in_class_udp_server_struct.cpp udp_utils.cpp)

add_executable(simple_udp_client ${CLIENT_SOURCE})
add_executable(simple_udp_server ${SERVER_SOURCE})

add_executable(simple_udp_client_struct ${CLIENT_STRUCT_SOURCE})
add_executable(simple_udp_server_struct ${SERVER_STRUCT_SOURCE})