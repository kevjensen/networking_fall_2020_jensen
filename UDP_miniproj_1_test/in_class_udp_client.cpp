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

  // IPv4 structure representing and IP address and port of the destination
  struct sockaddr_in dest_addr;

  // Set dest_addr to all zeroes, just to make sure it's not filled with junk
  // Note we could also make it a static variable, which will be zeroed before execution
  memset(&dest_addr, 0, sizeof(struct sockaddr_in));

  // Note: this needs to be 4, because the program name counts as an argument!
  if (argc < 4) {
    std::cerr << "Please specify IP PORT DATA as first three arguments." << std::endl;
    return 1;
  }
  // Set up variables "aliases"
  ip_string = argv[1];
  port_string = argv[2];
  data_string = argv[3];

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
  ret = sendto(udp_socket, data_string, strlen(data_string), 0,
               (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in));

  // Check if send worked, clean up and exit if not.
  if (ret <= 0) {
    handle_error("Sendto failed");
    close(udp_socket);
    return 1;
  }

  std::cout << "Sent " << ret << " bytes out." << std::endl;

  /**
   * Code to receive response from the server goes here!
   * recv or recvfrom...
   */

  close(udp_socket);
  return 0;
}