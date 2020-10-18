#include <iostream>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "udp_utils.h"
#include "tictactoe.h"

/**
 * Entrypoint to the program.
 *
 * @param argc count of arguments on command line
 * @param argv character array of command line arguments
 *
 * @return exit code of the program
 */
int main(int argc, char *argv[]) {
  /* alias for command line argument for ip address */
  char *ip_str;
  /* alias for command line argument for port */
  char *port_str;
  /* udp_socket will be the socket used for sending/receiving */
  int udp_socket;
  /* port will be the integer value of the port */
  unsigned int port;

  struct TTTMessage to_receive;

  struct GetGameMessage get_game_message;

  /* Dest contains the IP address and port in binary format for bind() */
  struct sockaddr_in dest;

  /* buffer to use for receiving data */
  static char recv_buf[2048];

  /* recv_addr is the client who is talking to us */
  struct sockaddr_in recv_addr;
  /* recv_addr_size stores the size of recv_addr */
  socklen_t recv_addr_size;
  /* buffer to use for sending data */
  static char send_buf[2048];
  /* variable to hold return values from network functions */
  int ret;

  if (argc < 3) {
    std::cerr << "Provide IP PORT as first two arguments." << std::endl;
    return 1;
  }
  /* assign ip_str to the first command line argument */
  ip_str = argv[1];
  /* assign port_str to the second command line argument */
  port_str = argv[2];

  // 1. Create the socket
  udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  // 2. Bind the socket

  // 2.1 convert ip address to binary form, store in dest
  ret = inet_pton(AF_INET, ip_str, &dest.sin_addr);
  if (ret == -1) {
    handle_error("inet_pton conversion failed");
    return 1;
  }

  // 2.2 convert port into binary form, store in dest

  ret = sscanf(port_str, "%u", &port);
  if (ret != 1) {
    std::cout << "Failed to convert port properly." << std::endl;
    return 1;
  }

  dest.sin_port = htons(port);
  // 2.3 set address type of dest
  dest.sin_family = AF_INET;

  ret = bind(udp_socket, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));

  if (ret == -1) {
    handle_error("bind failed");
    return 1;
  }

  // 3. Receive data
  recv_addr_size = sizeof(struct sockaddr_in);
  ret = recvfrom(udp_socket, recv_buf, 2047, 0, (struct sockaddr *)&recv_addr, &recv_addr_size);

  if (ret <= 0) {
    handle_error("recvfrom failed for some reason");
    close(udp_socket);
    return 1;
  }

  std::cout << "Received " << ret << " bytes of data." << std::endl;
  if (ret >= sizeof(struct TTTMessage)) {
    memcpy(&to_receive, recv_buf, sizeof(struct TTTMessage));
    to_receive.type = ntohs(to_receive.type);
    to_receive.len = ntohs(to_receive.len);
    std::cout << "received message type " << to_receive.type << " length " << to_receive.len << std::endl;

    if ((to_receive.type == ClientGetGame) && (ret >= sizeof(struct GetGameMessage)))
    {
      memcpy(&get_game_message, recv_buf, sizeof(struct GetGameMessage));
      get_game_message.hdr.type = ntohs(get_game_message.hdr.type);
      get_game_message.hdr.len = ntohs(get_game_message.hdr.len);
      get_game_message.client_id = ntohs(get_game_message.client_id);

      std::cout << "received GetGameMessage, type " << get_game_message.hdr.type << " length "
                << get_game_message.hdr.len << " client_id " << get_game_message.client_id << std::endl;
    }
  }
  //recv_buf[ret] = '\0';
  //std::cout << "Data received was `" << recv_buf << "'" << std::endl;

  // 4. Send back data
  ret = sendto(udp_socket, recv_buf, ret, 0, (struct sockaddr *)&recv_addr, recv_addr_size);

  if (ret <= 0) {
    handle_error("sendto failed for some reason");
    close(udp_socket);
    return 1;
  }

  std::cout << "sent " << ret << " bytes to client" << std::endl;
  close(udp_socket);
  return 0;
}
