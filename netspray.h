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

#define NETSPRAY_STATE_DEFAULT_BUFFER_SIZE 1024

struct netspray_state
{
  // ==== begin user mutable ====
  char *addr_str; // cstring of host to get addr info of
  char *port_str; // cstring of port to connect to

  char stream; // 0 if we only one to send once with a connection
  char write; // 1 if we want to send() data
  char read; // 1 if we want to recv() data
  char read_async; // 1 if we want to fork to recv() data

  struct addrinfo *addr; // just init this to NULL cus i'm an idiot

  // ==== end user mutable ====
  // ...unless you really want to ;)
  char reading;

  int sockfd;

  char *buffer;
  size_t buffer_size;

};

// creates a socket, writes buffer_size bytes to it, closes socket
// returns number of bytes written to socket, or error
// will keep a connection open after writing, if desired (stream = 1)
// can read from a connection if desired (via fork() child)
int netspray(char *buffer, size_t buffer_size, struct netspray_state *this);

// writes the given number of bytes to the socket
int netspray_write_bytes(char *buffer, size_t buffer_size, int sockfd);

// reads until heath death of socket (todo feature will fix this)
int netspray_read_bytes(char *buffer, size_t buffer_size, int sockfd);

// creates a new connection from a given state
int netspray_new_connection(struct netspray_state *this);

// when we're done with a connection
void netspray_cleanup(struct netspray_state *this);

// create an "empty" struct netspray_state
struct netspray_state netspray_new_null_state();

// create a new state for a given addr/port
struct netspray_state netspray_new_state();

// create a netspray_state with the given params
struct netspray_state netspray_new_state_wrappee(char *addr_str_in, char *port_str_in,
                                                 char stream_in, char write_in, char read_in,
                                                 char read_async_in, struct addrinfo *addr_in,
                                                 char reading_in, int sockfd_in, char *buffer_in,
                                                 size_t buffer_size_in);

#endif
