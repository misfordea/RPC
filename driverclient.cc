//
//  driverclient.cc
//  rpc
//
//  Created by Mdea
//

#include <iostream>
#include "remotefilesystem.h"
#include "connection.h"
#include "communication.h"

using namespace std;

const char *PROG_NAME = "none";


int main(int argc, char*argv[]) {
  PROG_NAME = argv[0];

  struct timeval length;
  length.tv_sec = 0;
  length.tv_usec = 100000;

//  string s = "localhost";
//  char* host = const_cast<char*>(s.c_str());
  char* host = argv[1];
  short port = atoi(argv[2]);
  // Referenced from http://coliru.stacked-crooked.com/a/ee6a28b593f88ca7
  // Generates 64 bit random number since getrandom(2) not available
  std::random_device rand;
  std::mt19937_64 e2(rand());
  std::uniform_int_distribution<long long int> u_dist(std::llround(std::pow(2,61)), std::llround(std::pow(2,62)));

  string a = "r+";
  char * mode1 = const_cast<char*>(a.c_str());

  string c = "w+";
  char * mode3 = const_cast<char*>(c.c_str());

  long long int auth = u_dist(e2);
  RemoteFileSystem rfs(host,port,auth,&length);
  RemoteFileSystem::File* f = rfs.open("test.txt", mode1);
  RemoteFileSystem::File* f2 = rfs.open("test2.txt", mode3);


  char buf[10];
  ssize_t readresult = f->read(buf, 10);
  cout << "readresult = " << readresult << endl;

  ssize_t readresult2 = f2->read(buf, 10);
  cout << "readresult2 = " << readresult2 << endl;

//  f->lseek(0, Communication::seek_set);
  char buf2[10];
  buf2[0] = 't';
  buf2[1] = '\n';
  buf2[2] = 't';
  buf2[3] = '\n';
  buf2[4] = 't';
  buf2[5] = '\n';
  ssize_t writeresult = f->write(buf2, 6);
  cout << "writeresult = " << writeresult << endl;

  ssize_t writeresultf2 = f2->write(buf2, 6);
  cout << "writeresultf2 = " << writeresultf2 << endl;

  off_t off_f2 = f2->lseek(0, Communication::seek_set);
  cout << "off_f2 = " << off_f2 << endl;

  ssize_t readresultf2 = f2->read(buf, 10);
  cout << "readresultf2 = " << readresultf2 << endl;

  int renameres = rfs.rename("test.txt", "testrename.txt");
  cout << "rename res = " << renameres << endl;

  char buf3[10];
  buf3[0] = 'f';
  buf3[1] = '\n';
  buf3[2] = 'u';
  buf3[3] = '\n';
  buf3[4] = 'n';
  buf3[5] = '\n';

  off_t lseekresult = f->lseek(0, Communication::seek_set);
  cout << "lseekresult = " << lseekresult << endl;

  ssize_t writeresult2 = f->write(buf3 , 6);
  cout << "writeresult2 = " << writeresult2 << endl;

  ssize_t writeresult3 = f->write(buf3 , 6);
  cout << "writeresult3 = " << writeresult3 << endl;

  ssize_t writeresult4 = f->write(buf3 , 6);
  cout << "writeresult4 = " << writeresult4 << endl;

  int renameres2 = rfs.rename("testrename.txt", "test.txt");
  cout << "renameres2 = " << renameres2 << endl;

  int unlinkres = rfs.unlink("test.txt");
  cout << "unlinkres = " << unlinkres << endl;

  string b = "w";
  char * mode2 = const_cast<char*>(b.c_str());
  mode_t m = (0666);
  RemoteFileSystem::File* f3 = rfs.open("test3.txt", mode2);

  off_t offsetre = f3->lseek(0, Communication::seek_set);
  cout << "offsetre = " << offsetre << endl;

  int chm = rfs.chmod("test3.txt", m);
  cout << "chm = " << chm << endl;

  RemoteFileSystem::File* f4 = rfs.open("test.txt", mode3);
  ssize_t writeresults5 = f4->write(buf2, 6);
  cout << "write results 5 = " << writeresults5 << endl;
}
