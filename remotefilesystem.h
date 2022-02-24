//
// RemoteFileSystem.h
//
// Client-side remote (network) filesystem
//
// Author: Morris Bernstein
// Copyright 2019, Systems Deployment, LLC.

#if !defined(RemoteFileSystem_H)
#define RemoteFileSystem_H


#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <iostream>
#include "connection.h"
#include "communication.h"

#define MAX_MSG_SIZE   4096

class RemoteFileSystem {
 public:

  // File represents an open file object
  class File {
  public:
    // Destructor closes open file.
    ~File();

   ssize_t read(void *buf, size_t count);
   ssize_t write(void *buf, size_t count);
   off_t lseek(off_t offset, int whence);

  private:
    // Only RemoteFileSystem can open a file.
    friend class RemoteFileSystem;
    File(RemoteFileSystem* filesystem, const char *pathname, char *mode);
    RemoteFileSystem* rfs;
    const char* filepath;
    char* mode;
    int fd;
    bool is_timeout_retry;

    // Disallow copy & assignment
    File(File const &) = delete;
    void operator=(File const &) = delete;
  };

  // Connect to remote system.  Throw error if connection cannot be
  // made.
  RemoteFileSystem(char *host,
       short port,
       unsigned long auth_token,
       struct timeval *timeout);

  // Disconnect
  ~RemoteFileSystem();

  // Return new open file object.  Client is responsible for
  // deleting.
  File *open(const char *pathname, char *mode);

  int chmod(const char *pathname, mode_t mode);
  int unlink(const char *pathname);
  int rename(const char *oldpath, const char *newpath);

 private:
  // File class may use private methods of the RemoteFileSystem to
  // implement their operations.  Alternatively, you can create a
  // separate Connection class that is local to your implementation.
  friend class File;
  Connection connector;
  std::vector<File*> open_files;
  
  char *host;
  short port;
  unsigned long auth_token;
  struct timeval *timeout;
  bool is_timeout_retry;
  
  // Disallow copy & assignment
  RemoteFileSystem(RemoteFileSystem const &) = delete;
  void operator=(RemoteFileSystem const &) = delete;
};


#endif
