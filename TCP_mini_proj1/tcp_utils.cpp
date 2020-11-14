#include "tcp_utils.h"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <iostream>

/**
 * Print an error related to networking to stderr using perror()
 * @param context some string about where the error occurred
 */
void handle_error(const char *context) {
  perror(context);
  return;
}

int convert_ip_port_to_sockaddr_in(char *ip_str, char *port_str, struct sockaddr_in *result) {
  unsigned int port;
  int ret;

  // 2.1 convert ip address to binary form, store in dest
  ret = inet_pton(AF_INET, ip_str, &result->sin_addr);
  if (ret == -1) {
    return -1;
  }

  // 2.2 convert port into binary form, store in dest

  ret = sscanf(port_str, "%u", &port);
  if (ret != 1) {
    return -1;
  }

  result->sin_port = htons(port);
  // 2.3 set address type of dest
  result->sin_family = AF_INET;

  return 0;
}

const char *printable_address(struct sockaddr_storage *client_addr, socklen_t client_addr_len) {
  // Buffer will be big enough for either a v4 or v6 address
  // AND big enough to put :65535 (the port) at the end.
  static char print_buf[NI_MAXHOST + NI_MAXSERV];
  static char host_buf[NI_MAXHOST];
  static char port_buf[NI_MAXSERV];

  int ret;
  // Verify address family is either v4 or v6
  switch (client_addr->ss_family) {
    case AF_INET:
      break;
    case AF_INET6:
      break;
    default:
      return nullptr;
  }

  // If we get here, we're good to go!
  ret = getnameinfo((struct sockaddr *)client_addr, client_addr_len,
                    host_buf, NI_MAXHOST,
                    port_buf, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);
  if (ret != 0) {
    std::cout << "getnameinfo error " << gai_strerror(errno) << std::endl;
    return nullptr;
  }

  strncpy(print_buf, host_buf, NI_MAXHOST);
  print_buf[strlen(host_buf)] = ':';
  strncpy(&print_buf[strlen(host_buf) + 1], port_buf, NI_MAXSERV);

  return print_buf;
}