//
// Connection.h
//
// Connection object
//
// Author: Mdea
//

#ifndef connection_h
#define connection_h

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string>

#define BUFFSIZE 4096

class Connection{
public:
  Connection();
  ~Connection();
  
  // Sets up server connection
  void server_con(short port);
  
  // Sets up client connection
  void client_con(char* server, char* port);
  
  // Referenced from socket - udp code from class
  // Sends message to server
  ssize_t send_to_server(const char* msg);
  
  // Sends message to client
  ssize_t send_to_client(const char* msg);
  
  // Receives message on client
  ssize_t recv_on_client(char* buff, size_t size);
  
  // Receives message on server
  ssize_t recv_on_server(char* buff, size_t size);
  
  // Sets timeout - used by client
  int set_timout(struct timeval *timeout); 
  
private:
  struct sockaddr_in* servaddr;
  struct sockaddr_in cliaddr;
  int sockfd;
  const char* message;
  
};


#endif /* connection_h */
