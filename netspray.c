#include "netspray.h"

int netspray (char *buffer, size_t buffer_size, struct netspray_state *this)
{
  // we need a state to save to
  if (NULL == this)
  { perror("null netspray state"); return -1; }

  // TODO performance, we can check before we call the function (but do we care?)
  if (0 != netspray_new_connection(this)) { fprintf(stderr, "error: new connection\n"); return -1; }

  int bytes_written = 0;
  if (0 != this->write)
  {
    if (-1 == (bytes_written = netspray_write_bytes(buffer, buffer_size, this->sockfd)))
    { fprintf(stderr, "error: write bytes\n"); return -1; }
  }

  if (0 != this->read && 0 == this->reading)
  {
    // we only want one child reading
    // so read errors will be fatal for that connection
    this->reading = 1;

    if (1 == this->read_async)
    {
      int pid = fork();
      if (-1 == pid)
      { perror("read's fork()"); return -1; }
      else if (0 == pid)
      {
        // read to the death
        while (-1 != netspray_read_bytes(this->buffer, this->buffer_size, this->sockfd))
        { fprintf(stdout, "%s", this->buffer); }

        return -1;
      }
    }
    else
    { netspray_read_bytes(this->buffer, this->buffer_size, this->sockfd); }
  }

  if (0 == this->stream)
  { netspray_cleanup(this); }

  return bytes_written;
}

int netspray_write_bytes (char *buffer, size_t buffer_size, int sockfd)
{
  int bytes_written = 0;
  int bytes_written_temp = -1;

  while ((size_t) bytes_written < buffer_size)
  {
    if (-1 == (bytes_written_temp = send(sockfd, &(buffer[bytes_written]), buffer_size - bytes_written, 0)))
    {
      perror("send()");
      break;
    }

    bytes_written += bytes_written_temp;
  }

  return bytes_written;
}

// TODO do we need to read more than once?
int netspray_read_bytes (char *buffer, size_t buffer_size, int sockfd)
{
  int bytes_read = 0;

  if (-1 == (bytes_read = recv(sockfd, buffer, buffer_size-1, 0)))
  { perror("recv()"); }

  // make buffer valid cstring
  buffer[buffer_size-1] = '\0';

  return bytes_read;
}

int netspray_new_connection (struct netspray_state *this)
{
  struct addrinfo addr_hint;
  struct addrinfo *addr_list;

  // null addr means we make a new connection
  if (NULL == this->addr)
  {
    memset(&addr_hint, 0, sizeof(addr_hint));
    addr_hint.ai_family = AF_UNSPEC;
    addr_hint.ai_socktype = SOCK_STREAM;
    if (0 != getaddrinfo(this->addr_str, this->port_str, &addr_hint, &addr_list))
    {
      perror("getaddrinfo()");
      return -1;
    }

    // find a valid addr to connect to
    for (this->addr = addr_list; NULL != this->addr; this->addr = this->addr->ai_next)
    {
      if (-1 == (this->sockfd = socket(this->addr->ai_family, this->addr->ai_socktype, this->addr->ai_protocol)))
      {
        perror("socket()");
        continue;
      }
      if (-1 == connect(this->sockfd, this->addr->ai_addr, this->addr->ai_addrlen))
      {
        perror("connect()");
        continue;
      }

      // socket created and connected for addr
      break;
    }

    if (NULL == this->addr)
    {
      fprintf(stderr, "could not connect to any addr\n");
      return -1;
    }
  }

  // all peaches
  return 0;
}

void netspray_cleanup (struct netspray_state *this)
{
  if (this->addr_str) {
    free(this->addr_str);
    this->addr_str = NULL;
  }

  if (this->port_str) {
    free(this->port_str);
    this->port_str = NULL;
  }

  this->stream = 0;
  this->write = 0;
  this->read = 0;
  this->read_async = 0;

  if (this->addr) {
    freeaddrinfo(this->addr);
    this->addr = NULL;
  }

  this->reading = 0;

  close(this->sockfd);
  this->sockfd = -1;

  if (this->buffer) {
    free(this->buffer);
    this->buffer = NULL;
    this->buffer_size = 0;
  }
}

struct netspray_null_state () {
  struct netspray_state state;

  state.addr_str = NULL;
  state.port_str = NULL;

  state.stream = 0;
  state.write = 0;
  state.read = 0;
  state.read_async = 0;

  state.addr = NULL;

  state.reading = 0;

  state.sockfd = -1;

  state.buffer = NULL;
  state.buffer_size = 0;

  return state;
}

// TODO create flags so we can set options "cleanly" with 1 variable
struct netspray_state netspray_new_state (char *addr_str_in, char *port_str_in) {
  // placeholders until we support flags
  char stream_in = 0;
  char write_in = 0;
  char read_in = 0;
  char read_async_in = 0;

  char *addr_in = NULL;

  char reading_in = 0;

  int sockfd_in = -1;

  // use default buffer size until we get flags
  char *buffer_in = NULL;
  size_t buffer_size_in = NETSPRAY_STATE_DEFAULT_BUFFER_SIZE;
  if (NULL == (buffer_in = calloc(buffer_size_in, 1))) {
    perror("could no allocate memory for netspray_state buffer");
    abort();
  }

  return netspray_new_state_wrappee(addr_str_in, port_str_in, stream_in, write_in, read_in,
                                    read_async_in, addr_in, reading_in, sockfd_in, buffer_in,
                                    buffer_size_in);
}

// _in 'cus some of these collide with symbols (e.g. read, write)
struct netspray_state netspray_new_state_wrappee (char *addr_str_in, char *port_str_in,
                                                  char stream_in, char write, char read_in,
                                                  char read_async_in, struct addrinfo *addr_in,
                                                  char reading_in, int sockfd_in, char *buffer_in,
                                                  size_t buffer_size_in) {
  struct netspray_state state = netspray_null_state();

  state.addr_str = addr_str_in;
  state.port_str = port_str_in;

  state.stream = stream_in;
  state.write = write_in;
  state.read = read_in;
  state.read_async = read_async_in;

  state.addr = addr_in;

  state.reading = reading_in;

  state.sockfd = sockfd_in;

  state.buffer = buffer_in;
  state.buffer_size = buffer_size_in;

  return state;
}
