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

#define NETSPRAY_STATE_BUFFER_LEN 1024

struct netspray_state
{
  // ==== begin user mutable ====
  char *addr_str; // cstring of host to get addr info of
  char *port_str; // cstring of port to connect to

  char stream; // 0 if we only one to send once with a connection
  char write; // 1 if we want to send() data
  char read; // 1 if we want to recv() data

  size_t buffer_len;
  // ==== end user mutable ====
  // ...unless you really want to ;)

  struct addrinfo *addr;

  int sockfd;

  char buffer[NETSPRAY_STATE_BUFFER_LEN];

};

// creates a socket, writes bytes_len bytes to it, closes socket
// returns bytes written to socket, or error
// will keep a connection open after writing, if desired (stream = 1)
// can read from a connection, but not one that is kept open
int netspray(char *bytes, size_t bytes_len, struct netspray_state *this);

// writes the given number of bytes to the socket
int netspray_write_bytes(char *bytes, size_t bytes_len, int sockfd);

// reads until heath death of socket (todo feature will fix this)
int netspray_read_bytes(char *buffer, size_t buffer_len, int sockfd);

// creates a new connection from a given state
int netspray_new_connection(struct netspray_state *this);

// when we're done with a connection
void netspray_cleanup(struct netspray_state *this);

#endif
