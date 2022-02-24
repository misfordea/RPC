//
// Communication.cc
//
// Communication object for message passing
//
// Author: Michelle Dea
//

#include "communication.h"

Communication::Communication() {
  
  // Referenced from http://coliru.stacked-crooked.com/a/ee6a28b593f88ca7
  // Generates 64 bit random number since getrandom(2) not available
  std::random_device rand;
  std::mt19937_64 e2(rand());
  std::uniform_int_distribution<long long int> u_dist(std::llround(std::pow(2,61)), std::llround(std::pow(2,62)));
  
  seq_no = u_dist(e2);
  file = "";
  new_path = "";
  fd = 0;
  request_results = "";
  
  auth_token = 0;
  call = OPEN;
  read_amt = 0;
  write_amt = 0;
  offset = 0;
  whence = seek_cur;
  mode = "";
  chmod_mode = 0;
}

Communication::~Communication() {
}

Communication* Communication::deserialize(const char* msg) {
  // Messages sent to server follow same format: Auth_token, Seq_no, File, Call, fd
  // Each element is separated with a new line \n
  std::string s(msg);

  std::istringstream msg_parse(s);

  try {
  
    std::string line;
    std::getline(msg_parse, line);
    auth_token = std::stoll(line);
    
    std::getline(msg_parse, line);
    seq_no = std::stoll(line);
    
    std::getline(msg_parse, line);
    
    std::string file_tmp = line;
    file = file_tmp;
    
    std::getline(msg_parse, line);
    call = static_cast<CALL>(stoi(line));
    
    std::getline(msg_parse, line);
    fd = stoi(line);
    

    
    switch (call) {
      case READ:
        std::getline(msg_parse, line);
        read_amt = static_cast<ssize_t>(stoi(line));
        break;
        
      case WRITE:
        std::getline(msg_parse, line);
        write_amt = static_cast<ssize_t>(stoi(line));
        break;
        
      case LSEEK:
        std::getline(msg_parse, line);
        offset = static_cast<off_t>(stoi(line));
        
        std::getline(msg_parse, line);
        whence = static_cast<Whence>(stoi(line));
        
        break;
        
      case OPEN:
      {
        std::getline(msg_parse, line);
        std::string tmp = line;
        
        mode = tmp;
        
        break;
      }
        
      case CHMOD:
        std::getline(msg_parse, line);
        chmod_mode = stoi(line);
        break;
        
      case RENAME:
      {
        std::getline(msg_parse, line);
        std::string np_tmp = line;
        new_path = np_tmp.c_str();
        break;
      }
        
      default:
        break;
    }

  }
  catch (std::exception& e)
  {
    call = INVALID;
    std::cout << "message invalid format for deserialization" << std::endl;
  }
  
  return this;
}

Communication* Communication::deserialize_response(const char* msg, size_t size) {
  std::string s(msg);

  // First line in message from server will be seq number
  std::istringstream msg_parse(s);
  std::string line;
  std::getline(msg_parse, line);
  std::string sq(line);
  seq_no = std::stoll(line);
  
  request_results = s.substr(sq.length()+1);
  
  return this;
}

std::string Communication::serialize() {
  // Message sent will be auth_token, seq_no, file, call, and fd
  std::string s = std::to_string (auth_token) + "\n" + std::to_string(seq_no) + "\n" + file + "\n" + std::to_string(call) + "\n" + std::to_string(fd);
  
  // Depending on the call type, additional info will be sent as follows:
  switch (call) {
    case READ:
      s += ("\n" + std::to_string(read_amt));
      break;
    case WRITE:
      s += ("\n" + std::to_string(write_amt));
      break;
    case LSEEK:
      s += ("\n" + std::to_string(offset) + "\n" + std::to_string(whence));
      break;
    case OPEN:
      s += ("\n" + static_cast<std::string>(mode));
      break;
    case CHMOD:
      //NEED TO FIX THE MODE - HOW TO SEND WITH THE |
      
      s += ("\n" + std::to_string(chmod_mode));
      break;
    case RENAME:
    {
      std::string tmp(new_path);
      s += ("\n" + tmp);
      break;
    }
      
    default:
      break;
  }
  
  s += "\n";
  
  return s;
}

void Communication::set_auth_token(long long int auth) {
  auth_token = auth;
}

void Communication::set_call(CALL c) {
  call = c;
}

void Communication::set_file(const char* f) {
  file = f;
}

void Communication::set_read_amt(ssize_t r) {
  read_amt = r;
}

void Communication::set_write_amt(ssize_t w) {
  write_amt = w;
}

void Communication::set_offset(off_t o) {
  offset = o;
}

void Communication::set_whence(Whence wh) {
  whence = wh;
}

void Communication::set_mode(char* m) {
  mode = m;
}

void Communication::set_chmod_mode(mode_t m) {
  chmod_mode = m;
}

void Communication::set_new_path(const char* np) {
  new_path = np;
}

void Communication::set_fd(int fd) {
  this->fd = fd;
}

void Communication::set_request_results(char* req) {
  request_results = req;
}

long long int Communication::get_seq_no() {
  return seq_no;
}

int Communication::get_call() {
  return call;
}

std::string Communication::get_file() {
  return file;
}

ssize_t Communication::get_read_amt() {
  return read_amt;
}

ssize_t Communication::get_write_amt() {
  return write_amt;
}

off_t Communication::get_offset() {
  return offset;
}

Communication::Whence Communication::get_whence() {
  return whence;
}

std::string Communication::get_mode() {
  return mode;
}

mode_t Communication::get_chmod_mode() {
  return chmod_mode;
}

std::string Communication::get_new_path() {
  return new_path;
}

int Communication::get_fd() {
  return fd;
}

std::string Communication::get_request_results() {
  return request_results;
}
