//
// Created by Marco De Mattia on 12/1/16.
//

#include "DatagramServerWithPoll.h"
#include <iostream>


DatagramServerWithPoll::DatagramServerWithPoll(const char *PORT, const char *UDP_PORT)
{
  int rv;
  struct addrinfo hints, *ai, *p;

  // get us a socket and bind it
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;      // AF_UNSPEC means automatically select between IPv4 or IPv6, whichever works.
  hints.ai_socktype = SOCK_STREAM;  // Stream type is TCP, UDP would be SOCK_DGRAM.
  hints.ai_flags = AI_PASSIVE;      // If node is null it allows to bind to this socket.
  if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
    fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
    exit(1);
  }

  // ai is a linked list of addrinfo structs that contain the socket address structures.
  for (p = ai; p != NULL; p = p->ai_next) {
    listener_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener_ < 0) {
      continue;
    }

    // lose the pesky "address already in use" error message
    setsockopt(listener_, SOL_SOCKET, SO_REUSEADDR, &yes_, sizeof(int));

    if (bind(listener_, p->ai_addr, p->ai_addrlen) < 0) {
      close(listener_);
      continue;
    }

    break;
  }

  // if we got here, it means we didn't get bound
  if (p == NULL) {
    fprintf(stderr, "selectserver: failed to bind\n");
    exit(2);
  }

  // Free the memory from the addrinfo linked list.
  freeaddrinfo(ai); // all done with this

  // listen: marks the socket referred to by sockfd as a passive socket,
  // that is, as a socket that will be used to accept incoming connection
  // requests using accept(). The second argument is the maximum backlog
  // of connections allowed on this socket.
  if (listen(listener_, 10) == -1) {
    perror("listen");
    exit(3);
  }

  pollfd ufdsListener;
  ufdsListener.fd = listener_;
  ufdsListener.events = POLLIN; // check for just normal data
  ufds_.push_back(ufdsListener);


  // Create the UPD socket where the input will be echoed to.
  // --------------------------------------------------------
  // get us a socket and bind it
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;      // AF_UNSPEC means automatically select between IPv4 or IPv6, whichever works.
  hints.ai_socktype = SOCK_DGRAM;  // Stream type is TCP, UDP would be SOCK_DGRAM.
  hints.ai_flags = AI_PASSIVE;      // If node is null it allows to bind to this socket.
  if ((rv = getaddrinfo(NULL, UDP_PORT, &hints, &ai)) != 0) {
    fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
    exit(1);
  }
  // ai is a linked list of addrinfo structs that contain the socket address structures.
  for (p = ai; p != NULL; p = p->ai_next) {
    broadcast_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (broadcast_ < 0) {
      continue;
    }
    break;
  }

  // if we got here, it means we didn't get bound
  if (p == NULL) {
    fprintf(stderr, "selectserver: failed to bind\n");
    exit(2);
  }

  broadcastServerInfo_ = p;
  // Free the memory from the addrinfo linked list.
  // freeaddrinfo(ai); // all done with this
}


void DatagramServerWithPoll::loop()
{
  int rv, i;
  // main loop
  for (;;) {
    // Check if any file descriptor has changed.
    rv = poll(ufds_.data(), nfds_t(ufds_.size()), -1);

    if (rv == -1) {
      perror("poll"); // error occurred in poll()
    } else if (rv == 0) {
      printf("Timeout occurred!  No data after 3.5 seconds.\n");
    } else {
      // check for events on listener socket:
      if (ufds_[0].revents & POLLIN) {
        addrlen_ = sizeof remoteaddr_;
        newfd_ = accept(listener_,
                        (struct sockaddr *) &remoteaddr_,
                        &addrlen_);
        if (newfd_ == -1) {
          perror("accept");
        } else {
          pollfd newClient;
          newClient.fd = newfd_;
          newClient.events = POLLIN; // check for just normal data
          ufds_.push_back(newClient);
          printf("selectserver: new connection from %s on "
                     "socket %d\n",
                 inet_ntop(remoteaddr_.ss_family,
                           get_in_addr((struct sockaddr *) &remoteaddr_),
                           remoteIP_, INET6_ADDRSTRLEN),
                 newfd_);
        }
      } else {
        for (i = 1; i < ufds_.size(); ++i) {
          if (ufds_[i].revents & POLLIN) {
            if ((nbytes_ = recv(ufds_[i].fd, buf_, sizeof buf_, 0)) <= 0) {
              if (nbytes_ == 0) {
                // connection closed
                printf("selectserver: socket %d hung up\n", i);
              } else {
                perror("recv");
              }
              close(ufds_[i].fd); // bye!
              ufds_.erase(ufds_.begin() + i);
              // Since we removed this element all the others move back by one.
              --i;
            } else {
              // we got some data from a client
              // Broadcast it to the UDP port
              // std::cout << "Sending message: \"" << buf_ << "\" to socket " << broadcast_ << std::endl;
              // std::cout << "broadcastServerInfo_->ai_addr = " << broadcastServerInfo_->ai_addr << std::endl;
              // std::cout << "broadcastServerInfo_->ai_addrlen = " << broadcastServerInfo_->ai_addrlen << std::endl;
              if ((nbytes_ = sendto(broadcast_, buf_, strlen(buf_), 0,
                                    broadcastServerInfo_->ai_addr, broadcastServerInfo_->ai_addrlen)) == -1) {
                perror("talker: sendto");
                exit(1);
              }
            }
          }
        } // END handle data from client
      } // END got new incoming connection
    } // END looping through file descriptors
  } // END for(;;)--and you thought it would never end!
}