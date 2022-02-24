//
// ServerFileSystem.cc
//
// Server-side filesystem
//
// Author: Michelle Dea
//

#include "serverfilesystem.h"

ServerFileSystem::ServerFileSystem(short port) {
  this->port = port;
  server_connector.server_con(port);
  start_server();
  
}

void ServerFileSystem::start_server() {

  for (;;) {

    char *msg = new char[MAX_MSG_SIZE];
    // Waits to receive a message from client
    ssize_t rr = server_connector.recv_on_server(msg, MAX_MSG_SIZE);
    if (rr < 0) {
      continue;
    }
    
    Communication c;
    c.deserialize(msg);
    
    memset(msg,0,MAX_MSG_SIZE);
    delete [] msg;
    
    // Based on call, will execute underlying system call
    // and send a message back to client with results
    switch (c.get_call())
    {
      
      //READ
      case Communication::READ:
      {
        ssize_t read_amt = c.get_read_amt();
        char* read_bytes = new char[read_amt];
        ssize_t result = read(c.get_fd(), read_bytes, read_amt);
        
        if (result < 0) {
          perror("error reading file");
        }
        
        std::string seq(std::to_string(c.get_seq_no()));
        std::string rb(read_bytes);
        
        // For READ, sends 2 messages
        // First message will be content that was read
        seq += ("\n" + rb);
        const char* read_msg = seq.c_str();
        server_connector.send_to_client(read_msg);
        
        // Second message will be bytes that were read
        std::string s = std::to_string(c.get_seq_no()) + "\n" + std::to_string(result);
        const char* read_results = s.c_str();
        server_connector.send_to_client(read_results);
        
        delete [] read_bytes;
        break;
        
      }
      //WRITE
      case Communication::WRITE:
      {
        
        ssize_t write_amt = c.get_write_amt();
        char* write_bytes = new char[write_amt];
        
        // Waits for a second message of what to write
        server_connector.recv_on_server(write_bytes, write_amt+1);
        ssize_t result = write(c.get_fd(), write_bytes, write_amt);
        if (result < 0) {
          perror("error writing to file ");
        }
        
        std::string s = std::to_string(c.get_seq_no()) + "\n" + std::to_string(result);
        const char* write_results = s.c_str();
        
        server_connector.send_to_client(write_results);
        
        delete [] write_bytes;
        break;
      }
      
      //LSEEK
      case Communication::LSEEK:
      {
        
        off_t result = lseek(c.get_fd(), c.get_offset(), c.get_whence());
        if (result < 0) {
          perror("error in lseek");
        }
        
        std::string s = std::to_string(c.get_seq_no()) + "\n" + std::to_string(result);
        const char* lseek_results = s.c_str();
        server_connector.send_to_client(lseek_results);
        break;
      }
        
      //OPEN
      case Communication::OPEN:
      {
        
        std::string modeName = c.get_mode();
        char* mode = const_cast<char*>(modeName.c_str());
        std::string fileName = c.get_file();
        const char* name = fileName.c_str();
//        std::string m(c.get_mode());
//        std::string n(c.get_file());
//        char* mode = const_cast<char*>(m.c_str());
//        const char* name = n.c_str();
        
        if ( *mode!= 'r' && *mode != 'w') {
          perror("mode must be r, r+, w, or w+");
        }
        
        int fd = -1;
        
        // Opens file using flags based on fopen man page
        if (*mode == 'w') {
          if (*(mode+1) == '+') {
            fd = open(name,O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
          } else {
            fd = open(name,O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
          }
        } else if (*mode == 'r') {
          if (*(mode+1) == '+') {
            fd = open(name,O_RDWR, S_IRUSR | S_IWUSR);
          } else {
            fd = open(name,O_RDONLY, S_IRUSR);
          }
        }
        
        if (fd == -1) {
          perror("error opening file");
        }
        
        std::string s = std::to_string(c.get_seq_no()) + "\n" + std::to_string(fd);
        const char* open_results = s.c_str();

        server_connector.send_to_client(open_results);

        break;
      }
      
      //CLOSE
      case Communication::CLOSE:
      {
        int result = close(c.get_fd());
        if (result < 0) {
          perror("error closing file");
        }
        break;
      }
      
      //CHMOD
      case Communication::CHMOD:
      {

        std::string n = c.get_file();
        const char* name = n.c_str();
        int result = chmod(name, c.get_chmod_mode());
        if (result < 0) {
          perror("error chmod");
        }
        std::string s = std::to_string(c.get_seq_no()) + "\n" + std::to_string(result);
        const char* chmod_results = s.c_str();
        server_connector.send_to_client(chmod_results);
        break;
      }
      
      //UNLINK
      case Communication::UNLINK:
      {

        std::string n = c.get_file();
        const char* name = n.c_str();
        int result = unlink(name);
        if (result < 0) {
          perror("error unlinking file");
        }
        std::string s = std::to_string(c.get_seq_no()) + "\n" + std::to_string(result);
        const char* unlink_results = s.c_str();
        server_connector.send_to_client(unlink_results);
        break;
      }
      
      //RENAME
      case Communication::RENAME:
      {

        std::string op = c.get_file();
        std::string np(c.get_new_path());
        const char* oldpath = op.c_str();
        const char* newpath = np.c_str();
        int result = rename(oldpath, newpath);
        if (result < 0) {
          perror("error renaming file");
        }
        std::string s = std::to_string(c.get_seq_no()) + "\n" + std::to_string(result);
        const char* rename_results = s.c_str();
        server_connector.send_to_client(rename_results);
        break;
      }
      
      //default
      default:
        break;
    }
  }
}

