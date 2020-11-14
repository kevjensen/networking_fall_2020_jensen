#ifndef IN_CLASS_UDP_EXAMPLE_UDP_UTILS_H
#define IN_CLASS_UDP_EXAMPLE_UDP_UTILS_H

#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>

/**
 * Print an error related to networking to stderr using perror()
 * @param context some string about where the error occurred
 */
void handle_error(const char *context);

int convert_ip_port_to_sockaddr_in(char *ip_str, char *port_str, struct sockaddr_in *result);

const char *printable_address(struct sockaddr_storage *client_addr, socklen_t client_addr_len);

#endif //IN_CLASS_UDP_EXAMPLE_UDP_UTILS_H