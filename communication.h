//
// Communication.h
//
// Communication object for message passing
//
// Author: Michelle Dea
//

#ifndef communication_h
#define communication_h

#include <sys/stat.h>
#include <string>
#include <string.h>
#include <iostream>
#include <random>
#include <cmath>
#include <sstream>
#include <exception>

class Communication {
public:
  Communication();
  ~Communication();
  
  // Deserializes message to server
  Communication* deserialize(const char* msg);
  
  // Deserializes response from server
  Communication* deserialize_response(const char* msg, size_t);
  
  // Serializes message to send to server
  std::string serialize();
  
  enum CALL : int {
    OPEN,
    READ,
    WRITE,
    LSEEK,
    CLOSE,
    CHMOD,
    UNLINK,
    RENAME,
    INVALID
  };
  
  enum Whence : int {
    seek_set,
    seek_cur,
    seek_end
  };
  
  void set_auth_token(long long int auth);
  void set_call(CALL c);
  void set_file(const char* f);
  void set_read_amt(ssize_t r);
  void set_write_amt(ssize_t w);
  void set_offset(off_t o);
  void set_whence(Whence wh);
  void set_mode(char* m);
  void set_chmod_mode(mode_t m);
  void set_new_path(const char* np);
  void set_fd(int fd);
  void set_request_results(char* req);
  
  long long int get_seq_no();
  int get_call();
  std::string  get_file();
  ssize_t get_read_amt();
  ssize_t get_write_amt();
  off_t get_offset();
  Whence get_whence();
  std::string get_mode();
  mode_t get_chmod_mode();
  std::string get_new_path();
  int get_fd();
  std::string get_request_results();
  
private:
  
  long long int auth_token;
  long long int seq_no;
  CALL call;   
  std::string file;
  ssize_t read_amt;
  ssize_t write_amt;
  off_t offset;
  Whence whence;
  std::string mode;
  mode_t chmod_mode;
  std::string new_path;
  int fd;
  std::string request_results;
  
};

#endif
