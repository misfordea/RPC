//
//  driverserver.cc
//  rpc
//
//  Created by Mdea
//

#include <iostream>
#include "serverfilesystem.h"
#include "connection.h"
#include "communication.h"

using namespace std;

const char *PROG_NAME = "none";

int main(int argc, char*argv[]) {
  PROG_NAME = argv[0];
  short port = atoi(argv[1]);
  ServerFileSystem sfs(port);


}
