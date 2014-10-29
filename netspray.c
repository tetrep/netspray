#include "netspray.h"

int netspray(char *bytes, size_t bytes_len, struct netspray_state *this)
{
  if(NULL == this)
  {
    perror("null state");
    return -1;
  }

  struct addrinfo addr_hint;
  struct addrinfo *addr_list;

  int bytes_written = 0;
  int bytes_written_temp = -1;

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

    //find a valid addr to connect to
    for(this->addr = addr_list; this->addr != NULL; this->addr = this->addr->ai_next)
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

      //socket created and connected for addr
      break;
    }

    if(NULL == this->addr)
    {
      perror("could not connect to any addr");
      return -1;
    }
  }
  //printf("%s", bytes);
  while(bytes_written < bytes_len)
  {
    if(-1 == (bytes_written_temp = send(this->sockfd, &(bytes[bytes_written]), bytes_len - bytes_written, 0)))
    {
      perror("send()");
      break;
    }

    bytes_written += bytes_written_temp;
  }

  if(this->read != 0)
  {
    bytes_written_temp = 0;
    //while(bytes_written_temp <= 0)
    {
      if(-1 == (bytes_written_temp = recv(this->sockfd, this->buffer, this->buffer_max_size-1, 0)))
      {
        perror("recv()");
      }
      else
      {
        fprintf(stderr, "received %i bytes:\n%s", bytes_written_temp, this->buffer);
      }
    }
  }

  if(0 == this->stream)
  {
    netspray_cleanup(this);
  }

  return bytes_written;
}

void netspray_cleanup(struct netspray_state *this)
{
  this->addr = NULL;

  close(this->sockfd);
  this->sockfd = -1;
}
