//
// Created by Nathan Evans
//
#include <iostream>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <netdb.h>

#include "tcp_chat.h"
#include "tcp_utils.h"

bool quit = false;

std::string get_nickname() {
	std::string nickname;
	std::cout << "Enter chat nickname: ";
	std::getline(std::cin, nickname);
	return nickname;
}

std::string get_message() {
	std::string msg;
	std::cout << "Enter chat message to send, or quit to quit: ";
	std::getline(std::cin, msg);
	//std::cerr << "Got input " << msg << " from user" << std::endl;
	return msg;
}

// Handler for when ctrl+c is pressed.
// Just set the global 'stop' to true to shut down the server.
void handle_ctrl_c(int the_signal) {
	std::cout << "Handled sigint\n";
	quit = true;
}

/**
 *
 * Chat client example. Reads in HOST PORT
 *
 * e.g., ./tcpchatclient 127.0.0.1 8888
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

	// The socket used to connect/send/receive data to the TCP server
	int client_socket;
	// Variable used to check return codes from various functions
	int ret;

	char send_buf[2048];

	std::string nickname;

	// Note: this needs to be 3, because the program name counts as an argument!
	if (argc < 3) {
		std::cerr << "Please specify HOST PORT as first two arguments." << std::endl;
		return 1;
	}
	// Set up variables "aliases"
	ip_string = argv[1];
	port_string = argv[2];

	// Signal handler setup, done for you! This allows you to hit ctrl+c when running from the command line
	// This will set the global quit variable to true and allow you to cleanly shut down from the program
	// E.g., send a disconnect message to the server and then close the TCP connection
	struct sigaction ctrl_c_handler;
	ctrl_c_handler.sa_handler = handle_ctrl_c;
	sigemptyset(&ctrl_c_handler.sa_mask);
	ctrl_c_handler.sa_flags = 0;
	sigaction(SIGINT, &ctrl_c_handler, NULL);

	// Create the TCP socket.
	// AF_INET is the address family used for IPv4 addresses
	// SOCK_STREAM indicates creation of a TCP socket
	client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Make sure socket was created successfully, or exit.
	if (client_socket == -1) {
		std::cerr << "Failed to create tcp socket!" << std::endl;
		std::cerr << strerror(errno) << std::endl;
		return 1;
	}

	// TODO: Get the IP address in binary format using get getaddrinfo!
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
		perror("getaddrinfo error.");
		return -1;
	}

	// TODO: Connect to TCP Chat Server using connect()
	results_it = results;
	ret = -1;
	while (results_it != NULL) {
		std::cout << "Attempting to connect to " <<
		          printable_address((struct sockaddr_storage *)results_it->ai_addr, results_it->ai_addrlen) << std::endl;

		ret = connect(client_socket, results->ai_addr, results->ai_addrlen);
		if (ret == 0) // Success
		{
			break;
		}
		ret = -1;
		perror("connect");
		results_it = results_it->ai_next;
	}
	// Whatever happened, we need to free the address list.
	freeaddrinfo(results);

	if (ret != 0) {
		std::cerr << "Failed to connect to chat server!" << std::endl;
		std::cerr << strerror(errno) << std::endl;
		close(client_socket);
		return 1;
	}

	struct ChatClientMessage *client_message;
	client_message = (struct ChatClientMessage *) malloc(sizeof(ChatClientMessage));
	memset(client_message, 0, sizeof(client_message));

	// TODO: Send connect message
	// Fill in client_message and send to the server
	client_message->type = htons(CLIENT_CONNECT);
	client_message->data_length = htons(sizeof(struct ChatClientMessage));

	ret = send(client_socket, client_message, sizeof(client_message), 0);

	if (ret <= 0) {
		handle_error("Connect send to server failed.");
		close(client_socket);
		return 1;
	}

	nickname = get_nickname();

	// TODO: Send nickname message
	struct ChatClientMessage *client_nickname;
	client_nickname = (struct ChatClientMessage *) malloc(sizeof(ChatClientMessage) + nickname.length());
	memset(client_nickname, 0, sizeof(client_nickname));
	int nick_name_msg_size = 0;
	nick_name_msg_size += sizeof(ChatClientMessage);
	nick_name_msg_size += nickname.length();

	client_nickname->type = htons(CLIENT_SET_NICKNAME);
	client_nickname->data_length = htons(nickname.size());

	int offset = 0;
	memcpy(&send_buf[offset], client_nickname, sizeof(struct ChatClientMessage));
	offset += sizeof(struct ChatClientMessage);
	memcpy(&send_buf[offset], nickname.c_str(), nickname.size());

	ret = send(client_socket, send_buf, nick_name_msg_size, 0);

	if (ret <= 0) {
		handle_error("Nickname message failed.");
		close(client_socket);
		return 1;
	}
	// Now enter a loop to send the chat messages from this client to the server
	std::string next_message;
	next_message = get_message();

	while ((next_message != "quit") && (quit == false)) {
		std::cout << "Sending message " << next_message << std::endl;
		// TODO: parse command from next_message, either a regular message, a direct message, or a LIST message
		//       then send to the server the correct message type and data based on that

		if (next_message[0] == '/') {
			std::string direct_nickname;
			for (int i = 0; i < next_message.size(); ++i) {
				if (next_message[i] == '/') {
					break;
				} else {
					direct_nickname.push_back(next_message[i]);
				}
			}
			// Send direct message; append name to message
			struct ChatClientMessage *client_direct_message;
			client_direct_message = (struct ChatClientMessage *) malloc(sizeof(ChatClientMessage));
			memset(client_direct_message, 0, sizeof(client_direct_message));

			client_direct_message->type = htons(CLIENT_SEND_DIRECT_MESSAGE);
			client_direct_message->data_length = htons(sizeof(client_direct_message));

			int offset = 0;
			char *client_direct_message_ptr = (char *) client_direct_message;
			offset += sizeof(struct ChatClientMessage);
			memcpy(&client_direct_message_ptr[offset], &next_message, next_message.size());
			offset += next_message.size();
			memcpy(&client_direct_message_ptr[offset], &direct_nickname, direct_nickname.size());

			ret =send(client_socket, client_direct_message, sizeof(client_direct_message), 0);

			if (ret <= 0) {
				handle_error("Client Direct Message failed.");
			}
		}
		else if (next_message == "LIST") {
			struct ChatClientMessage *client_list_message;
			client_list_message = (struct ChatClientMessage *) malloc(sizeof(ChatClientMessage));
			memset(client_message, 0, sizeof(client_list_message));

			client_list_message->type = htons(CLIENT_GET_MEMBERS);
			client_message->data_length = htons(sizeof(struct ChatClientMessage));
			ret = send(client_socket, client_list_message, sizeof(client_list_message), 0);
			std::cout << "Sent LIST Message" << std::endl;

			if (ret <= 0) {
				handle_error("Client LIST message failed.");
				close(client_socket);
				return 1;
			}

		} else {
			struct ChatClientMessage *client_normal_message;
			client_normal_message = (struct ChatClientMessage *) malloc(sizeof(ChatClientMessage));
			memset(client_message, 0, sizeof(client_normal_message));

			client_normal_message->type = htons(CLIENT_SEND_MESSAGE);
			client_message->data_length = htons(sizeof(struct ChatClientMessage));

			int offset = 0;
			char *client_message_ptr = (char *)client_normal_message;
			offset += sizeof(struct ChatClientMessage);
			memcpy(&client_message_ptr[offset], &next_message, next_message.size());

			ret = send(client_socket, client_normal_message, sizeof(client_normal_message), 0);

			if (ret <= 0) {
				handle_error("Send normal message failed.");
				close(client_socket);
				return 1;
			}

		}

		next_message = get_message();
	}

	// TODO: build and send a client disconnect message to the server here

		struct ChatClientMessage *client_quit_message;
		client_quit_message = (struct ChatClientMessage *) malloc(sizeof(ChatClientMessage));
		memset(client_message, 0, sizeof(client_quit_message));

		client_quit_message->type = htons(CLIENT_DISCONNECT);
		client_message->data_length = htons(sizeof(struct ChatClientMessage));
		ret = send(client_socket, client_message, sizeof(client_quit_message), 0);

		if (ret <= 0) {
			handle_error("Disconnect from server failed.");
			close(client_socket);
			return 1;
		}

		std::cout << nickname << " disconnected from server." <<  std::endl;

	close(client_socket);
	return 0;
}
