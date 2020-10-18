#include "udp_utils.h"
#include <arpa/inet.h>

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