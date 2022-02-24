//
// ServerFileSystem.h
//
// Server-side filesystem
//
// Author: Michelle Dea
//

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <iostream>
#include "connection.h"
#include "communication.h"

#define MAX_MSG_SIZE   4096

#ifndef serverfilesystem_h
#define serverfilesystem_h

class ServerFileSystem {
public:
  // Creates connection object for server
  ServerFileSystem(short port);
  
  // Starts server
  void start_server();
  
private:
  Connection server_connector;
  short port;
};

#endif /* serverfilesystem_h */
