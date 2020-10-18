all: ttt_client udpserver

udpserver: in_class_udp_server_struct.cpp udp_utils.cpp udp_utils.h
	g++ in_class_udp_server_struct.cpp udp_utils.cpp -o udpserver

ttt_client: in_class_udp_client_struct.cpp udp_utils.cpp udp_utils.h
	g++ in_class_udp_client_struct.cpp udp_utils.cpp -o ttt_client

