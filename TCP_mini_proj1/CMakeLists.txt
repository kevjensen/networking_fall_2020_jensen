cmake_minimum_required(VERSION 3.17)
project(TCP_mini_proj1)

set(CMAKE_CXX_STANDARD 14)

set(TCP_CLIENT_SOURCE tcp_chat_client.cpp tcp_utils.cpp tcp_chat.h)
set(TCP_MONITOR_SOURCE tcp_chat_monitor.cpp tcp_utils.cpp tcp_chat.h)

#add_executable(TCP_mini_proj1 tcp_chat_client.cpp tcp_chat_monitor.cpp tcp_chat.h)
add_executable(tcp_chat_client.cpp ${TCP_CLIENT_SOURCE})
add_executable(tcp_chat_monitor.cpp ${TCP_MONITOR_SOURCE})
