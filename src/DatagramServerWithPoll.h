//
// Created by Marco De Mattia on 12/1/16.
//

#ifndef SOCKETS_DATAGRAMSERVERWITHPOLL_H
#define SOCKETS_DATAGRAMSERVERWITHPOLL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <vector>

/**
 * Simple server that listens to connection on a TCP port (default 9034) and echoes the input
 * to a UDP port (default 10024). The UDPClient is able to listen to this UDP port and print
 * the messages.
 */


class DatagramServerWithPoll {
 public:
  DatagramServerWithPoll(const char * PORT, const char * UDP_PORT);
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
  int listener_;     // listening socket descriptor
  int broadcast_;    // UDP socket where the input will be echoed to
  struct addrinfo * broadcastServerInfo_;
  int newfd_;        // newly accept()ed socket descriptor
  struct sockaddr_storage remoteaddr_; // client address
  socklen_t addrlen_;

  char buf_[256];    // buffer for client data
  int nbytes_;

  char remoteIP_[INET6_ADDRSTRLEN];

  int yes_=1;        // for setsockopt() SO_REUSEADDR, below

  std::vector<pollfd> ufds_;
};


#endif //SOCKETS_DATAGRAMSERVERWITHPOLL_H
