#include "netspray.h"

int netspray(char *bytes, size_t bytes_len, struct netspray_state *this)
{
  // we need a state to save to
  if(NULL == this)
  { perror("null netspray state"); return -1; }

  if(0 != netspray_new_connection(this)){perror("new connection"); return -1;}

  int bytes_written = 0;
  if(0 != this->write)
  {
    if(-1 == (bytes_written = netspray_write_bytes(bytes, bytes_len, this->sockfd)))
    { perror("write bytes"); return -1; }
  }

  // we can only read when we aren't streaming because we're dumb
  if(0 != this->read && 0 == this->stream)
  {
    if(-1 == netspray_read_bytes(this->buffer, this->buffer_len, this->sockfd))
    { perror("read bytes"); return -1; }
  }

  if(0 == this->stream)
  { netspray_cleanup(this); }

  return bytes_written;
}

int netspray_write_bytes(char *bytes, size_t bytes_len, int sockfd)
{
  int bytes_written = 0;
  int bytes_written_temp = -1;

  // printf("%s", bytes);
  while(bytes_written < bytes_len)
  {
    if(-1 == (bytes_written_temp = send(sockfd, &(bytes[bytes_written]), bytes_len - bytes_written, 0)))
    {
      perror("send()");
      break;
    }

    bytes_written += bytes_written_temp;
  }

  return bytes_written;
}

int netspray_read_bytes(char *buffer, size_t buffer_len, int sockfd)
{
  int bytes_read = 0;
  int bytes_read_temp = 0;

  while(bytes_read_temp >= 0)
  {
    if(-1 == (bytes_read_temp = recv(sockfd, buffer, buffer_len-1, 0)))
    {
      perror("recv()");
      break;
    }
    else
    {
      // ensure buffer is valid cstring before printing
      buffer[buffer_len-1] = '\0';
      fprintf(stderr, "received %i bytes:\n%s", bytes_read_temp, buffer);
    }

    bytes_read += bytes_read_temp;
  }

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
      perror("could not connect to any addr");
      return -1;
    }
  }

  // all peaches
  return 0;
}

void netspray_cleanup(struct netspray_state *this)
{
  this->addr = NULL;

  close(this->sockfd);
  this->sockfd = -1;
}
