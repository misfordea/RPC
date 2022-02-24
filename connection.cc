//
// Connection.cc
//
// Connection object
//
// Author: Michelle Dea
//

#include "connection.h"

Connection::Connection() {
  // Open socket
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) {
    perror("connection error - socket creation failed");
  }
}

Connection::~Connection() {
  // Close socket
  close(sockfd);
}

void Connection::server_con(short port) {
  //referenced from udp-server.c provided by Professor Morris Bernstein
  struct sockaddr_in server_addr;
  bzero(&server_addr, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(port);
  
  int br = bind(sockfd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in));
  if (br < 0) {
    perror("server_con error: bind failed");
  }
}

void Connection::client_con(char* server, char *port) {
  //referenced from udp-client.c provided by Professor Morris Bernstein
  struct addrinfo hints;
  bzero(&hints, sizeof(struct addrinfo));
  hints.ai_flags = 0;
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = 0;
  hints.ai_addrlen = 0;
  hints.ai_addr = NULL;
  hints.ai_canonname = NULL;
  hints.ai_next = NULL;

  struct addrinfo *addresses;

  int result = getaddrinfo(server, port, &hints, &addresses);
  if (result != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
  }
  
  servaddr = (struct sockaddr_in *) addresses->ai_addr;
  
}

ssize_t Connection::send_to_server(const char* msg) {
  //referenced from udp-server.c provided by Professor Morris Bernstein
  message = msg;
  
  ssize_t result = 0;
  
  result = sendto(sockfd, (void *) message, strlen(message), 0, (struct sockaddr *) servaddr, sizeof(struct sockaddr_in));
  
  if (result < 0) {
    perror("error sending to server");
  }
  
  return result;
  
}

ssize_t Connection::send_to_client(const char* msg) {
  //referenced from udp-client.c provided by Professor Morris Bernstein
  message = msg;
  
  ssize_t result = 0;
  
  result = sendto(sockfd, (void *) message, strlen(message), 0, (struct sockaddr *) &cliaddr, sizeof(struct sockaddr_in));
  
  if (result < 0) {
    perror("error sending to client");
  }
  
  return result;
  
}

ssize_t Connection::recv_on_client(char* buff, size_t size) {
  //referenced from udp-client.c provided by Professor Morris Bernstein
  ssize_t result = 0;
  
  result = recv(sockfd, buff, size-1, 0);
  
  if (result < 0) {
    perror("error recv_client");
  }
  
  return result;
}

ssize_t Connection::recv_on_server(char* buff, size_t size) {
  //referenced from udp-server.c provided by Professor Morris Bernstein
  ssize_t result = 0;
  socklen_t len = sizeof(struct sockaddr_in);
  
  result = recvfrom(sockfd, buff, size-1, 0, (struct sockaddr *) &cliaddr, &len);
  
  if (result < 0) {
    perror("error recv_server");
  }
  
  return result;
  
  
}

int Connection::set_timout(struct timeval *timeout) {
  // Sets up timeout for receiving messages - will be used by client
  int result = setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,timeout,sizeof(struct timeval));
  
  if (result < 0) {
      perror("error with timeout");
  }
  
  return result;
}
