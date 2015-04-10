#include "netspray.h"

int netspray(char *buffer, size_t buffer_size, struct netspray_state *this)
{
  // we need a state to save to
  if(NULL == this)
  { perror("null netspray state"); return -1; }

  if(0 != netspray_new_connection(this)) { fprintf(stderr, "error: new connection\n"); return -1; }

  int bytes_written = 0;
  if(0 != this->write)
  {
    if(-1 == (bytes_written = netspray_write_bytes(buffer, buffer_size, this->sockfd)))
    { fprintf(stderr, "error: write bytes\n"); return -1; }
  }

  if(0 != this->read && 0 == this->reading)
  {
    // we only want one child reading
    // so read errors will be fatal for that connection
    this->reading = 1;

    // not sure the best spot to set this
    this->buffer_size = NETSPRAY_STATE_BUFFER_SIZE;

    if (1 == this->read_async)
    {
      int pid = fork();
      if(-1 == pid)
      { perror("read's fork()"); return -1; }
      else if(0 == pid)
      {
        // read to the death
        while(-1 != netspray_read_bytes(this->buffer, this->buffer_size, this->sockfd))
        { fprintf(stdout, "%s", this->buffer); }

        return -1;
      }
    }
    else
    { netspray_read_bytes(this->buffer, this->buffer_size, this->sockfd); }
  }

  if(0 == this->stream)
  { netspray_cleanup(this); }

  return bytes_written;
}

int netspray_write_bytes(char *buffer, size_t buffer_size, int sockfd)
{
  int bytes_written = 0;
  int bytes_written_temp = -1;

  while((size_t) bytes_written < buffer_size)
  {
    if(-1 == (bytes_written_temp = send(sockfd, &(buffer[bytes_written]), buffer_size - bytes_written, 0)))
    {
      perror("send()");
      break;
    }

    bytes_written += bytes_written_temp;
  }

  return bytes_written;
}

// TODO do we need to read more than once?
int netspray_read_bytes(char *buffer, size_t buffer_size, int sockfd)
{
  int bytes_read = 0;

  if(-1 == (bytes_read = recv(sockfd, buffer, buffer_size-1, 0)))
  { perror("recv()"); }

  // make buffer valid cstring
  buffer[buffer_size-1] = '\0';

  return bytes_read;
}

int netspray_new_connection(struct netspray_state *this)
{
  struct addrinfo addr_hint;
  struct addrinfo *addr_list;

  // null addr means we make a new connection
  if(NULL == this->addr)
  {
    memset(&addr_hint, 0, sizeof(addr_hint));
    addr_hint.ai_family = AF_UNSPEC;
    addr_hint.ai_socktype = SOCK_STREAM;
    if(0 != getaddrinfo(this->addr_str, this->port_str, &addr_hint, &addr_list))
    {
      perror("getaddrinfo()");
      return -1;
    }

    // find a valid addr to connect to
    for(this->addr = addr_list; NULL != this->addr; this->addr = this->addr->ai_next)
    {
      if(-1 == (this->sockfd = socket(this->addr->ai_family, this->addr->ai_socktype, this->addr->ai_protocol)))
      {
        perror("socket()");
        continue;
      }
      if(-1 == connect(this->sockfd, this->addr->ai_addr, this->addr->ai_addrlen))
      {
        perror("connect()");
        continue;
      }

      // socket created and connected for addr
      break;
    }

    if(NULL == this->addr)
    {
      fprintf(stderr, "could not connect to any addr\n");
      return -1;
    }
  }

  // all peaches
  return 0;
}

void netspray_cleanup(struct netspray_state *this)
{
  this->addr = NULL;
  this->reading = 0;

  close(this->sockfd);
  this->sockfd = -1;
}
