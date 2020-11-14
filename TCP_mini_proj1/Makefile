all: tcpchatmon tcpchatcli

tcpchatcli:tcp_chat_client.cpp tcp_chat.h tcp_utils.cpp tcp_utils.h
	g++ tcp_chat_client.cpp tcp_utils.cpp -o tcpchatcli

tcpchatmon: tcp_chat_monitor.cpp tcp_chat.h tcp_utils.cpp tcp_utils.h
	g++ tcp_chat_monitor.cpp tcp_utils.cpp -o tcpchatmon