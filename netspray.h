#ifndef NETSPRAY_HEADER
#define NETSPRAY_HEADER

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>

struct netspray_state
{
  char *addr_str;
  char *port_str;

  struct addrinfo *addr;

  int sockfd;

  char buffer[1024];
  size_t buffer_max_size;

  char stream;
  char read;
};

//creates a socket, writes bytes_len bytes to it, closes socket
//returns bytes written to socket, or error
int netspray(char *bytes, size_t bytes_len, struct netspray_state *this);

void netspray_cleanup(struct netspray_state *this);

#endif
