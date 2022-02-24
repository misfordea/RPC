//
// RemoteFileSystem.cc
//
// Client-side remote (network) filesystem
//
// Author: Michelle Dea
//

#include "remotefilesystem.h"


RemoteFileSystem::File::File(RemoteFileSystem* filesystem, const char *pathname, char *mode) {
  rfs = filesystem;
  filepath = pathname;
  this->mode = mode;
  
}

RemoteFileSystem::File::~File(){
  // Sends a message to Server to close file
  Communication c;
  c.set_auth_token(rfs->auth_token);
  c.set_file(filepath);
  c.set_call(Communication::CLOSE);
  c.set_fd(fd);
  
  std::string ser = c.serialize();
  const char* msg = ser.c_str();
  rfs->connector.send_to_server(msg);
}

ssize_t RemoteFileSystem::File::read(void *buf, size_t count) {
  Communication c;
  c.set_auth_token(rfs->auth_token);
  c.set_file(filepath);
  c.set_call(Communication::READ);
  c.set_fd(fd);
  c.set_read_amt(count);
  
  // Sends message to server of how much to read
  std::string ser = c.serialize();
  const char* msg = ser.c_str();
  rfs->connector.send_to_server(msg);
  
  // Size of seq_no in bytes
  std::string sz = std::to_string(c.get_seq_no());
  
  // Receive message from server with what was read
  char* tmp = new char[count+1+(sz.length()+1)];
  // Set pointer to null
  memset(tmp,0,count+1+(sz.length()+1));
  
  auto recv_result = rfs->connector.recv_on_client(tmp, count+1+(sz.length()+1));
  
  char* tmp2 = reinterpret_cast<char*>(buf);
  
  // If error is due to timeout, sends message again
  if (recv_result < 0 && errno == EAGAIN && !is_timeout_retry) {
    is_timeout_retry = true;
    
    std::cout << "timed out, resending read message..." << std::endl;
    
    return read(buf, count);
  }
  
  is_timeout_retry = false;
  
  Communication r;
  r.deserialize_response(tmp, count);
  
  if (c.get_seq_no() != r.get_seq_no()) {
    std::cout << "msg returned: wrong seq no" << std::endl;
    return -1;
  }
  
  // Copies results of read to buf
  memcpy(tmp2, r.get_request_results().c_str(), count);
  
  // Receive message for amount of bytes read
  char* read_result = new char[MAX_MSG_SIZE];
  
  // Set pointer to null
  memset(read_result,0,MAX_MSG_SIZE);
  
  recv_result = rfs->connector.recv_on_client(read_result, MAX_MSG_SIZE);
  
  
  if (recv_result < 0 && errno == EAGAIN && !is_timeout_retry) {
    is_timeout_retry = true;

    std::cout << "timed out, read resending message..." << std::endl;
    
    return read(buf, count);
  }
  
  is_timeout_retry = false;
  
  Communication r2;
  r2.deserialize_response(read_result, MAX_MSG_SIZE);
  
  if (c.get_seq_no() != r2.get_seq_no()) {
    std::cout << "msg returned: wrong seq no" << std::endl;
    return -1;
  }
  
  std::string s = r2.get_request_results();
  
  ssize_t result = static_cast<ssize_t>(std::stoi(s));
  
  delete [] read_result;
  delete [] tmp;
  
  return result;
}

ssize_t RemoteFileSystem::File::write(void *buf, size_t count) {
  Communication c;
  c.set_auth_token(rfs->auth_token);
  c.set_file(filepath);
  c.set_call(Communication::WRITE);
  c.set_fd(fd);
  c.set_write_amt(count);
  
  // Sends message to server with how much to read
  std::string ser = c.serialize();
  const char* msg = ser.c_str();
  rfs->connector.send_to_server(msg);
  
  // Sends second message with what to write
  const char * bytes_to_write = reinterpret_cast<const char*>(buf);
  rfs->connector.send_to_server(bytes_to_write);

  // Receives message from server of how many bytes written
  char* write_result = new char[MAX_MSG_SIZE];
  
  memset(write_result,0,MAX_MSG_SIZE);
  
  auto recv_result = rfs->connector.recv_on_client(write_result, MAX_MSG_SIZE);
  
  // If error is due to timeout, sends message again
  if (recv_result < 0 && errno == EAGAIN && !is_timeout_retry) {
    is_timeout_retry = true;

    std::cout << "timed out, resending write message..." << std::endl;
    
    return write(buf, count);
  }
  
  is_timeout_retry = false;

  Communication r;
  r.deserialize_response(write_result, MAX_MSG_SIZE);
  
  if (c.get_seq_no() != r.get_seq_no()) {
    std::cout << "msg returned: wrong seq no" << std::endl;
    return -1;
  }
  
  std::string s = r.get_request_results();
  
  ssize_t result = static_cast<ssize_t>(std::stoi(s));
  
  delete [] write_result;
  
  return result;
}

off_t RemoteFileSystem::File::lseek(off_t offset, int whence) {
  Communication c;
  c.set_auth_token(rfs->auth_token);
  c.set_file(filepath);
  c.set_call(Communication::LSEEK);
  c.set_fd(fd);
  c.set_offset(offset);
  c.set_whence(static_cast<Communication::Whence>(whence));
  
  // Sends message to server of where to move file pointer
  std::string ser = c.serialize();
  const char* msg = ser.c_str();
  rfs->connector.send_to_server(msg);
  
  // Receives message of offset from beginning of file
  char* lseek_results = new char[MAX_MSG_SIZE];
  auto recv_result = rfs->connector.recv_on_client(lseek_results, MAX_MSG_SIZE);
  
  // If error is due to timeout, sends message again
  if (recv_result < 0 && errno == EAGAIN && !is_timeout_retry) {
    is_timeout_retry = true;

    std::cout << "timed out, resending lseek message..." << std::endl;
    
    return lseek(offset, whence);
  }
  
  is_timeout_retry = false;
  
  Communication r;
  r.deserialize_response(lseek_results, MAX_MSG_SIZE);
  
  if (c.get_seq_no() != r.get_seq_no()) {
    std::cout << "msg returned: wrong seq no" << std::endl;
    return -1;
  }
  
  std::string s = r.get_request_results();
  
  off_t result = static_cast<off_t>(std::stoi(s));
  
  delete [] lseek_results;
  
  return result;
}

RemoteFileSystem::RemoteFileSystem(char *host,
     short port,
     unsigned long auth_token,
     struct timeval *timeout) {
  
  this->host = host;
  this->port = port;
  this->auth_token = auth_token;
  this->timeout = timeout;
  
  // Set up connection object
  std::string s = std::to_string(port);
  char* port_c = const_cast<char*>(s.c_str());
  connector.client_con(host, port_c);
  
  // Set up timeout
  connector.set_timout(this->timeout);
}

// Disconnect
RemoteFileSystem::~RemoteFileSystem() {
  // Deletes open files
  for (auto file: open_files) {
    delete file;
  }
}

// Return new open file object.  Client is responsible for
// deleting.
RemoteFileSystem::File * RemoteFileSystem::open(const char *pathname, char *mode) {
  
  File* f = new File(this,pathname,mode);
  
  Communication c;
  c.set_auth_token(auth_token);
  c.set_file(pathname);
  c.set_call(Communication::OPEN);
  c.set_mode(mode);
  
  // Sends message to open file
  std::string ser = c.serialize();
  const char* msg = ser.c_str();
  
  connector.send_to_server(msg);
  
  // Adds new file f to open files vector
  open_files.push_back(f);
  
  char* open_results = new char[MAX_MSG_SIZE];
  auto recv_result = connector.recv_on_client(open_results, MAX_MSG_SIZE);
  
  // If error is due to timeout, sends message again
  if (recv_result < 0 && errno == EAGAIN && !is_timeout_retry) {
    is_timeout_retry = true;

    std::cout << "timed out, resending open message..." << std::endl;
    
    return open(pathname, mode);
  }
  
  is_timeout_retry = false;
  
  Communication r;
  
  r.deserialize_response(open_results, MAX_MSG_SIZE);
  
  if (c.get_seq_no() != r.get_seq_no()) {
    std::cout << "msg returned: wrong seq no" << std::endl;
    return NULL;
  }
  
  std::string s = r.get_request_results();

  // Sets fd in file
  f->fd = std::stoi(s);
  
  delete [] open_results;
  
  return f;
  
}

int RemoteFileSystem::chmod(const char *pathname, mode_t mode) {
  Communication c;
  c.set_auth_token(auth_token);
  c.set_file(pathname);
  c.set_call(Communication::CHMOD);
  c.set_chmod_mode(mode);
  
  // Sends message for chmod
  std::string ser = c.serialize();
  const char* msg = ser.c_str();
  connector.send_to_server(msg);
  
  char* chmod_results = new char[MAX_MSG_SIZE];
  auto recv_result = connector.recv_on_client(chmod_results, MAX_MSG_SIZE);
  
  // If error is due to timeout, sends message again
  if (recv_result < 0 && errno == EAGAIN && !is_timeout_retry) {
    is_timeout_retry = true;

    std::cout << "timed out, resending chmod message..." << std::endl;
    
    return chmod(pathname, mode);
  }
  
  is_timeout_retry = false;
  
  Communication r;
  r.deserialize_response(chmod_results, MAX_MSG_SIZE);
  
  if (c.get_seq_no() != r.get_seq_no()) {
    std::cout << "msg returned: wrong seq no" << std::endl;
    return -1;
  }
  
  std::string s = r.get_request_results();
  
  int result = std::stoi(s);
  
  delete [] chmod_results;
  
  return result;
}

int RemoteFileSystem::unlink(const char *pathname) {
  Communication c;
  c.set_auth_token(auth_token);
  c.set_file(pathname);
  c.set_call(Communication::UNLINK);
  
  // Sends message for which file to unlink
  std::string ser = c.serialize();
  const char* msg = ser.c_str();
  connector.send_to_server(msg);
  
  char* unlink_results = new char[MAX_MSG_SIZE];
  auto recv_result = connector.recv_on_client(unlink_results, MAX_MSG_SIZE);
  
  // If error is due to timeout, sends message again
  if (recv_result < 0 && errno == EAGAIN && !is_timeout_retry) {
    is_timeout_retry = true;

    std::cout << "timed out, resending unlink message..." << std::endl;
    
    return unlink(pathname);
  }
  
  is_timeout_retry = false;
  
  Communication r;
  r.deserialize_response(unlink_results, MAX_MSG_SIZE);
  
  if (c.get_seq_no() != r.get_seq_no()) {
    std::cout << "msg returned: wrong seq no" << std::endl;
    return -1;
  }
  
  std::string s = r.get_request_results();
  
  int result = std::stoi(s);
  
  delete [] unlink_results;
  
  return result;
}

int RemoteFileSystem::rename(const char *oldpath, const char *newpath){
  Communication c;
  c.set_auth_token(auth_token);
  c.set_file(oldpath);
  c.set_call(Communication::RENAME);
  c.set_new_path(newpath);
  
  // Sends message of what to rename file
  std::string ser = c.serialize();
  const char* msg = ser.c_str();
  connector.send_to_server(msg);
  
  char* rename_results = new char[MAX_MSG_SIZE];
  auto recv_result = connector.recv_on_client(rename_results, MAX_MSG_SIZE);
  
  // If error is due to timeout, sends message again
  if (recv_result < 0 && errno == EAGAIN && !is_timeout_retry) {
    is_timeout_retry = true;

    std::cout << "timed out, resending rename message..." << std::endl;
    
    return rename(oldpath, newpath);
  }
  
  is_timeout_retry = false;
  
  Communication r;
  r.deserialize_response(rename_results, MAX_MSG_SIZE);
  
  if (c.get_seq_no() != r.get_seq_no()) {
    std::cout << "msg returned: wrong seq no" << std::endl;
    return -1;
  }
  
  std::string s = r.get_request_results();
  
  int result = std::stoi(s);
  
  delete [] rename_results;
  
  return result;
}

