//
// Created by Marco De Mattia on 12/1/16.
//

#ifndef SOCKETS_SERVERWITHSELECT_H
#define SOCKETS_SERVERWITHSELECT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/**
 * Simple server that listens to connection on a TCP port (default 9034) and echoes the input
 * messages to the sockets connected on that port (via TCP). It works as a simple group chat in
 * that the input message from one socket is sent to all other sockets (except the origin of the
 * message). The input TCP port is monitored with the select() command.
 */


class StreamingServerWithSelect
{
 public:
  StreamingServerWithSelect(const char * PORT);
  void loop();

  // get sockaddr, IPv4 or IPv6:
  void *get_in_addr(struct sockaddr *sa)
  {
    if (sa->sa_family == AF_INET) {
      return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
  }

 private:
  fd_set master_;    // master file descriptor list
  fd_set read_fds_;  // temp file descriptor list for select()
  int fdmax_;        // maximum file descriptor number

  int listener_;     // listening socket descriptor
  int newfd_;        // newly accept()ed socket descriptor
  struct sockaddr_storage remoteaddr_; // client address
  socklen_t addrlen_;

  char buf_[256];    // buffer for client data
  int nbytes_;

  char remoteIP_[INET6_ADDRSTRLEN];

  int yes_=1;        // for setsockopt() SO_REUSEADDR, below

};


#endif //SOCKETS_SERVERWITHSELECT_H
