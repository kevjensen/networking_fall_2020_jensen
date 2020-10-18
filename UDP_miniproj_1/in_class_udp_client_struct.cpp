/**
 * In-class demonstrated UDP client example.
 */

#include <iostream>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <bitset>

#include "udp_utils.h"
#include "tictactoe.h"

/**
 *
 * Dead simple UDP client example. Reads in IP PORT DATA
 * from the command line, and sends DATA via UDP to IP:PORT.
 *
 * e.g., ./udpclient 127.0.0.1 8888 this_is_some_data_to_send
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
	char *data_string;
	// Port to send UDP data to. Need to convert from command line string to a number
	unsigned int port;
	// The socket used to send UDP data on
	int udp_socket;
	// Variable used to check return codes from various functions
	int ret;

	// To import data from the server
	struct TTTMessage to_receive;

	// Test struct to show how to send things over the network
	struct GetGameMessage to_send;

	//Send results of game back to server
	struct GameResultMessage result_send;

	// Declare Game
	Games game;

	// Send game result back to server
	struct GameSummaryMessage get_summary_message;

	// IPv4 structure representing and IP address and port of the destination
	struct sockaddr_in dest_addr;

	/* buffer to use for receiving data */
	static char recv_buf[2048];

	/* recv_addr is the client who is talking to us */
	struct sockaddr_in recv_addr;
	/* recv_addr_size stores the size of recv_addr */
	socklen_t recv_addr_size;

	// Set dest_addr to all zeroes, just to make sure it's not filled with junk
	// Note we could also make it a static variable, which will be zeroed before execution
	memset(&dest_addr, 0, sizeof(struct sockaddr_in));

	// Note: this needs to be 4, because the program name counts as an argument!
	if (argc < 3) {
		std::cerr << "Please specify IP PORT DATA as first three arguments." << std::endl;
		return 1;
	}
	// Set up variables "aliases"
	ip_string = argv[1];
	port_string = argv[2];

	// Create the UDP socket.
	// AF_INET is the address family used for IPv4 addresses
	// SOCK_DGRAM indicates creation of a UDP socket
	udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// Make sure socket was created successfully, or exit.
	if (udp_socket < 0) {
		handle_error("UDP socket creation failed.");
	}

	// inet_pton converts an ip address string (e.g., 1.2.3.4) into the 4 byte
	// equivalent required for using the address in code.
	// Note that because dest_addr is a sockaddr_in (again, IPv4) the 'sin_addr'
	// member of the struct is used for the IP
	// Check whether the specified IP was parsed properly. If not, exit.
	// Convert the port string into an unsigned integer.
	// sscanf is called with one argument to convert, so the result should be 1
	// If not, exit.
	// Set the address family to AF_INET (IPv4)
	// Set the destination port. Use htons (host to network short)
	// to ensure that the port is in big endian format

	ret = convert_ip_port_to_sockaddr_in(ip_string, port_string, &dest_addr);
	if (ret == -1) {
		handle_error("ip/port conversion failed");
	}


	// Send the data to the destination.
	// Note 1: we are sending strlen(data_string) (don't include the null terminator)
	// Note 2: we are casting dest_addr to a struct sockaddr because sendto uses the size
	//         and family to determine what type of address it is.
	// Note 3: the return value of sendto is the number of bytes sent
	//ret = sendto(udp_socket, data_string, strlen(data_string), 0,
	//             (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in));

	to_send.hdr.type = htons(ClientGetGame);
	to_send.hdr.len = htons(sizeof(struct GetGameMessage));
	to_send.client_id = htons(837);

	std::cout << "Will send 'GetGameMessage' via UDP to " << ip_string << ":" << port_string << std::endl;
	// Now we want to send to_send struct instead
	ret = sendto(udp_socket, &to_send, sizeof(struct GetGameMessage), 0,
	             (struct sockaddr *) &dest_addr, sizeof(struct sockaddr_in));

	// Check if send worked, clean up and exit if not.
	if (ret <= 0) {
		handle_error("Sendto failed");
		close(udp_socket);
		return 1;
	}

	std::cout << "Sent " << ret << " bytes via UDP." << std::endl;

	// Client needs to receive data, it should be a game summary message. Then verify the lengths and type
	// then pull out actual game stuff.

	recv_addr_size = sizeof(struct sockaddr_in);
	ret = recvfrom(udp_socket, recv_buf, 2047, 0, (struct sockaddr *) &recv_addr, &recv_addr_size);

	if (ret < 0) {
		handle_error("recvfrom failed for some reason");
		close(udp_socket);
		return 1;
	}

	std::cout << "Received " << ret << " bytes from " << ip_string << ":" << port_string << std::endl;
	if (ret >= sizeof(struct TTTMessage)) {
		memcpy(&to_receive, recv_buf, sizeof(struct TTTMessage));
		to_receive.type = ntohs(to_receive.type);
		to_receive.len = ntohs(to_receive.len);

		if ((to_receive.type == ServerInvalidRequestReply) && (ret >= sizeof(struct TTTMessage))) {
			std::cout << "Server error returned. You did something stupid.";
		} else if ((to_receive.type == ServerGameReply) && (ret >= sizeof(struct TTTMessage))) {
			memcpy(&get_summary_message, recv_buf, sizeof(struct GameSummaryMessage));
			get_summary_message.hdr.type = ntohs(get_summary_message.hdr.type);
			get_summary_message.hdr.len = ntohs(get_summary_message.hdr.len);
			get_summary_message.client_id = ntohs(get_summary_message.client_id);
			get_summary_message.game_id = ntohs(get_summary_message.game_id);
			get_summary_message.o_positions = ntohs(get_summary_message.o_positions);
			get_summary_message.x_positions = ntohs(get_summary_message.x_positions);

			std::cout << "Received game result length " << get_summary_message.hdr.len << " client ID "
			          << get_summary_message.client_id
			          << " game ID " << get_summary_message.game_id << "\nX positions:"
			          << get_summary_message.x_positions << ", "
			          << " O positions:" << get_summary_message.o_positions << "\n\n";
		}

		//Initialize board; I had undefined behavior if I did not initialize it. Probably some memory issues
		for (int i = 0; i < sizeof(game.board); ++i) {
			game.board[i] = ' ';
		}

		std::bitset<16> x_binary(get_summary_message.x_positions);
		std::bitset<16> o_binary(get_summary_message.o_positions);

		std::cout << "Tic Tac Toe\n\nPlayer 1 (X) - Player 2 (O)\n\n";

		// Check board validity.
		for (int i = 0; i < sizeof(game.board); ++i) {
			if (x_binary.test(i)) {
				if (game.board[i] == 'O') {
					game.result = INVALID_BOARD;
				} else {
					game.board[i] = 'X';
				}
			}
			if (o_binary.test(i)) {
				if (game.board[i] == 'X') {
					game.result = INVALID_BOARD;
				} else {
					game.board[i] = 'O';
				}
			}

		}

		// Print board. Normally I'd do sommething like this in a loop but it was much faster to do it this way.
		std::cout << "   |   |   \n"
		<< " " << game.board[0] << " | " << game.board[1] << " | " << game.board[2] <<
		"\n___|___|___\n" << "   |   |   \n"
		<< " " << game.board[3] << " | " << game.board[4] << " | " << game.board[5]
		<< "\n___|___|___\n" << "   |   |   \n" << " "
		<< game.board[6] << " | " << game.board[7] << " | " << game.board[8]
		<< "\n   |   |\n\n";

		//Rows
		if (game.result != INVALID_BOARD) {
			if (game.board[0] == 'X' && game.board[1] == 'X' && game.board[2] == 'X') {
				std::cout << "X is a winner!\n";
				game.result = X_WIN;
			} else if (game.board[3] == 'X' && game.board[4] == 'X' && game.board[5] == 'X') {
				std::cout << "X is a winner!\n";
				game.result = X_WIN;
			} else if (game.board[6] == 'X' && game.board[7] == 'X' && game.board[8] == 'X') {
				std::cout << "X is a winner!\n";
				game.result = X_WIN;
			} else if (game.board[0] == 'O' && game.board[1] == 'O' && game.board[2] == 'O') {
				std::cout << "O is a winner!\n";
				game.result = O_WIN;
			} else if (game.board[3] == 'O' && game.board[4] == 'O' && game.board[5] == 'O') {
				std::cout << "O is a winner!\n";
				game.result = O_WIN;
			} else if (game.board[6] == 'O' && game.board[7] == 'O' && game.board[8] == 'O') {
				std::cout << "O is a winner!\n";
				game.result = O_WIN;
			}

			//Columns
			else if (game.board[0] == 'X' && game.board[3] == 'X' && game.board[6] == 'X') {
				std::cout << "X is a winner!\n";
				game.result = X_WIN;
			} else if (game.board[1] == 'X' && game.board[4] == 'X' && game.board[7] == 'X') {
				std::cout << "X is a winner!\n";
				game.result = X_WIN;
			} else if (game.board[0] == 'X' && game.board[3] == 'X' && game.board[6] == 'X') {
				std::cout << "X is a winner!\n";
				game.result = X_WIN;
			} else if (game.board[0] == 'O' && game.board[3] == 'O' && game.board[6] == 'O') {
				std::cout << "O is a winner!\n";
				game.result = O_WIN;
			} else if (game.board[1] == 'O' && game.board[4] == 'O' && game.board[7] == 'O') {
				std::cout << "O is a winner!\n";
				game.result = O_WIN;
			} else if (game.board[2] == 'O' && game.board[5] == 'O' && game.board[8] == 'O') {
				std::cout << "O is a winner!\n";
				game.result = O_WIN;
			}

			//Diagonals
			else if (game.board[0] == 'X' && game.board[4] == 'X' && game.board[8] == 'X') {
				std::cout << "X is a winner!\n";
				game.result = X_WIN;
			} else if (game.board[2] == 'X' && game.board[4] == 'X' && game.board[6] == 'X') {
				std::cout << "X is a winner!\n";
				game.result = X_WIN;
			} else if (game.board[0] == 'O' && game.board[4] == 'O' && game.board[8] == 'O') {
				std::cout << "O is a winner!\n";
				game.result = O_WIN;
			} else if (game.board[2] == 'O' && game.board[4] == 'O' && game.board[6] == 'O') {
				std::cout << "O is a winner!\n";
				game.result = O_WIN;
			} else {
				std::cout << "Cat's game.\n";
				game.result = CATS_GAME;
			}
		} else {
			std::cout << "Invalid board from server.\n";
		}
	}

	//game.result = X_WIN;
	//std::cout << "Result sent to server: " << game.result << "\n";
	//Send result to server
	result_send.hdr.type = htons(ClientResult);
	result_send.hdr.len = htons(sizeof(struct GameResultMessage));
	result_send.game_id = htons(get_summary_message.game_id);
	result_send.result = htons(game.result);

	ret = sendto(udp_socket, &result_send, sizeof(struct GameResultMessage), 0,
	             (struct sockaddr *) &dest_addr, sizeof(struct sockaddr_in));

	// Check if send worked, clean up and exit if not.
	if (ret <= 0) {
		handle_error("Client result message failed.\n");
		close(udp_socket);
		return 1;
	}

	std::cout << "Sent " << ret << " bytes via UDP." << std::endl;

	//Confirm result from server
	recv_addr_size = sizeof(struct sockaddr_in);
	ret = recvfrom(udp_socket, recv_buf, 2047, 0, (struct sockaddr *) &recv_addr, &recv_addr_size);

	if (ret <= 0) {
		handle_error("recvfrom failed for some reason");
		close(udp_socket);
		return 1;
	}

	std::cout << "Received " << ret << " bytes from " << ip_string << ":" << port_string << std::endl;
	if (ret >= sizeof(struct TTTMessage)) {
		memcpy(&to_receive, recv_buf, sizeof(struct TTTMessage));
		to_receive.type = ntohs(to_receive.type);
		to_receive.len = ntohs(to_receive.len);

		if ((to_receive.type != ServerClientResultIncorrect && to_receive.type != ServerClientResultCorrect)) {
			std::cerr << "Server error returned. You did something stupid.";
			return 1;
		}
		if (to_receive.type == ServerClientResultCorrect) {
			std::cout << "Got CORRECT result from server!\n";
		}
		if (to_receive.type == ServerClientResultIncorrect) {
			std::cout << "Got INCORRECT result from server!\n";
		}
	}

	close(udp_socket);
	return 0;
}