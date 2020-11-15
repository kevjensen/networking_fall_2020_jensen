//
// Created by Nathan Evans
//
#include <iostream>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <netdb.h>

#include "tcp_chat.h"
#include "tcp_utils.h"

// Variable used to shut down the monitor when ctrl+c is pressed.
static bool stop = false;

// Handler for when ctrl+c is pressed.
// Just set the global 'stop' to true to shut down the server.
void handle_ctrl_c_monitor(int the_signal) {
	std::cout << "Handled sigint\n";
	stop = true;
}

/**
 * TCP chat monitor. Connects to a chat server and
 * simply prints out data to the client until it quits.
 *
 * e.g., ./tcpchatmon 127.0.0.1 8888
 *
 * @param argc count of arguments on the command line
 * @param argv array of command line arguments
 * @return 0 on success, non-zero if an error occurred
 */
int main(int argc, char *argv[]) {
	// Alias for argv[1] for convenience
	char *ip_string;
	// Alias for argv[2] for convenience
	char *port_string;
	// Alias for argv[3] for convenience
	char *nickname = nullptr;
	// The socket used to connect/send/recv data to the TCP chat server
	int monitor_socket;
	// Variable used to check return codes from various functions
	int ret;
	struct pollfd pfds[2];
	int stdin_fd = 0;

	// IPv4 structure representing and IP address and port of the destination
	struct sockaddr_in dest_addr;
	socklen_t dest_addr_len;
	char recv_buf[2048];
	char send_buf[2049];

	fd_set read_set; // fds to read from
	fd_set write_set; // fds to write to
	fd_set except_set; // fds with errors
	int max_fds; // highest fd + 1
	struct timeval select_timeout; // how long select will wait before returning
	select_timeout.tv_sec = 2;
	select_timeout.tv_usec = 0;

	// Set dest_addr to all zeroes, just to make sure it's not filled with junk
	// Note we could also make it a static variable, which will be zeroed before execution
	memset(&dest_addr, 0, sizeof(struct sockaddr_in));

	// Signal handler to deal with quitting the program appropriately
	struct sigaction ctrl_c_handler;
	ctrl_c_handler.sa_handler = handle_ctrl_c_monitor;
	sigemptyset(&ctrl_c_handler.sa_mask);
	ctrl_c_handler.sa_flags = 0;
	sigaction(SIGINT, &ctrl_c_handler, NULL);

	// Note: this needs to be 3, because the program name counts as an argument!
	if (argc < 3) {
		std::cerr << "Please specify server HOST PORT [NICKNAME] as arguments." << std::endl;
		return 1;
	}


	// Indicates that a nickname was provided for the monitor (for direct messages)
	if (argc == 4) {
		nickname = argv[3];
	}

	// Set up les variables "aliases"
	ip_string = argv[1];
	port_string = argv[2];

	// Create le socket
	monitor_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (monitor_socket < 0) {
		perror("socket");
		return -1;
	}

	// 2. Connect to chat server
	struct addrinfo hints;
	struct addrinfo *results;
	struct addrinfo *results_it;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_protocol = 0;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	ret = getaddrinfo(ip_string, port_string, &hints, &results);

	if (ret != 0) {
		perror("getaddrinfo");
		return -1;
	}

	results_it = results;
	ret = -1;
	while (results_it != NULL) {
		std::cout << "Attempting to connect to " <<
		          printable_address((struct sockaddr_storage *) results_it->ai_addr, results_it->ai_addrlen)
		          << std::endl;
		ret = connect(monitor_socket, results_it->ai_addr, results_it->ai_addrlen);
		if (ret == 0) {
			break;
		}
		ret = -1;
		perror("bind");
		results_it = results_it->ai_next;
	}

	// Free memory for a free... computer?
	freeaddrinfo(results);

	if (ret == -1) {
		handle_error("connect failed");
		return -1;
	}

	// Set flags to keep socket from blocking
	int flags;
	flags = fcntl(monitor_socket, F_GETFL, 0);

	if (flags == -1) {
		perror("fcntl");
		return -1;
	}

	ret = fcntl(monitor_socket, F_SETFL, flags | SOCK_NONBLOCK);
	if (ret == -1) {
		perror("fcntl");
		return -1;
	}

	// TODO: build a chat client message of type MON_CONNECT
	//       if a nickname was provided, include that in the message as well
	struct ChatMonMsg *mon_connect;
	mon_connect = (struct ChatMonMsg *) malloc(sizeof(ChatMonMsg));
	memset(mon_connect, 0, sizeof(ChatMonMsg));
	int mon_connect_size = 0;
	int offset = 0;

	mon_connect_size += sizeof(ChatMonMsg);

	// TODO: send the MON_CONNECT message to the server
	// Check if send worked, clean up and exit if not.
	mon_connect->type = htons(MON_CONNECT);

	if (nickname != nullptr) {
		mon_connect->nickname_len = htons(strlen(nickname));
		mon_connect_size += strlen(nickname);

		memcpy(&send_buf[offset], mon_connect, sizeof(struct ChatMonMsg));
		offset += sizeof(struct ChatMonMsg);
		memcpy(&send_buf[offset], nickname, strlen(nickname));
		ret = send(monitor_socket, send_buf, mon_connect_size, 0);
		std::cout << "Sent nickname connect." << std::endl;
	} else {
		ret = send(monitor_socket, mon_connect, mon_connect_size, 0);
	}

	if (ret <= 0) {
		handle_error("Connect send to server failed.");
		close(monitor_socket);
		return 1;
	}

	std::cout << "Mon connect message sent." << std::endl;

	// Placeholder for messages received from the server
	struct ChatMonMsg server_message;
	// After sending the connect monitor message, the monitor will just
	// sit and wait for messages to output.
	while (stop == false) {
		// Set up fd_sets
		FD_ZERO(&read_set);
		FD_ZERO(&write_set);
		FD_ZERO(&except_set);

		// Set file descriptors we care about
		FD_SET(monitor_socket, &read_set);
		max_fds = monitor_socket + 1;
		FD_SET(stdin_fd, &read_set);


		select_timeout.tv_sec = 2;
		select_timeout.tv_usec = 0;
		ret = select(max_fds, &read_set, NULL, NULL, &select_timeout);

		if (ret < 0) {
			perror("select");
			break;
		}

		// TODO: receive messages from the server
		//       when a message from the server is received, you should determine its type and data, then print
		//       out the chat message to the screen, including the nickname of the sender

		if (FD_ISSET(monitor_socket, &read_set)) {

			ret = recv(monitor_socket, recv_buf, 2047, 0);

			if (ret <= 0) {
				handle_error("recv failed for some reason");
				continue;
			}

			if (ret >= sizeof(struct ChatMonMsg)) {
				memcpy(&server_message, recv_buf, sizeof(struct ChatMonMsg));
				server_message.type = ntohs(server_message.type);
				server_message.data_len = ntohs(server_message.data_len);
				server_message.nickname_len = ntohs(server_message.nickname_len);

				if (server_message.type == MON_MESSAGE) {

					int offset = 0;
					offset += sizeof(struct ChatMonMsg);
					char *temp_data_buf = (char *) malloc(server_message.data_len + 1);
					char *temp_name_buf = (char *) malloc(server_message.nickname_len + 1);
					memset(temp_data_buf, 0, server_message.data_len);
					memset(temp_name_buf, 0, server_message.nickname_len);
					if (ret >= offset + server_message.nickname_len) {
						memcpy(temp_name_buf, &recv_buf[offset], server_message.nickname_len);
						recv_buf[server_message.nickname_len] = '\0';
						offset += server_message.nickname_len;
						std::cout << temp_name_buf << " said: ";
						memcpy(temp_data_buf, &recv_buf[offset], server_message.data_len);
						std::cout << temp_data_buf << std::endl;
					}
				}
				if (server_message.type == MON_DIRECT_MESSAGE) {

					int offset = 0;
					offset += sizeof(struct ChatMonMsg);
					char *temp_data_buf = (char *) malloc(server_message.data_len + 1);
					char *temp_name_buf = (char *) malloc(server_message.nickname_len + 1);
					memset(temp_data_buf, 0, server_message.data_len);
					memset(temp_name_buf, 0, server_message.nickname_len);
					if (ret >= offset + server_message.nickname_len) {
						memcpy(temp_name_buf, &recv_buf[offset], server_message.nickname_len);
						recv_buf[server_message.nickname_len] = '\0';
						offset += server_message.nickname_len;
						std::cout << "[DIRECT] " << temp_name_buf << " said: ";
						memcpy(temp_data_buf, &recv_buf[offset], server_message.data_len);
						std::cout << temp_data_buf << std::endl;

					}
				}
			}
		}
			// TODO: read from stdin, in case the user types 'quit'
			if (FD_ISSET(stdin_fd, &read_set)) {
				std::cout << "Have data incoming on stdin" << std::endl;
				ret = read(stdin_fd, recv_buf, 2047);
				if (ret > 0) {
					recv_buf[ret] = '\0';
					if (strncmp(recv_buf, "quit", 6)) {
						stop = true;
					}
				}
			}
		}

	// TODO: build and send a MON_DISCONNECT message to let the server know this monitor has gone away
	struct ChatMonMsg *mon_disconnect;
	mon_disconnect= (struct ChatMonMsg*) malloc(sizeof(struct ChatMonMsg));
	memset(mon_disconnect, 0, sizeof(struct ChatMonMsg));

	mon_disconnect->type = htons(MON_DISCONNECT);

	ret = send(monitor_socket, mon_disconnect, sizeof(struct ChatMonMsg), 0);

	if (ret <= 0) {
		perror("disconnect failed.");
	}

	std::cout << "Shut down message sent to server, exiting!\n";

	close(monitor_socket);
	return 0;

}

