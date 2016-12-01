//
// Created by Marco De Mattia on 12/1/16.
//

#include "ServerWithPoll.h"
#include <iostream>

ServerWithPoll::ServerWithPoll(const char *PORT)
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
}


void ServerWithPoll::loop()
{
  int rv, i, j;
  // main loop
  for (;;) {
    // Check if any file descriptor has changed.
//    std::cout << "total sockets = " << ufds_.size() << std::endl;
    rv = poll(ufds_.data(), nfds_t(ufds_.size()), -1);

//    std::cout << "rv = " << rv << std::endl;

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
//          std::cout << "checking socket " << i << " with fd = " << ufds_[i].fd << std::endl;

          if (ufds_[i].revents & POLLIN) {
//            std::cout << "ufds_[" << i << "]." << ufds_[i].fd << " nbytes = " << std::endl;
            if ((nbytes_ = recv(ufds_[i].fd, buf_, sizeof buf_, 0)) <= 0) {
//              std::cout << "ufds_[" << i << "]." << ufds_[i].fd << " nbytes = " << nbytes_ << std::endl;
//              printf(buf_);
              if (nbytes_ == 0) {
                // connection closed
                printf("selectserver: socket %d hung up\n", i);
              } else {
                perror("recv");
              }
//              std::cout << "closing " << ufds_[i].fd << std::endl;
              close(ufds_[i].fd); // bye!
              ufds_.erase(ufds_.begin() + i);
              // Since we removed this element all the others move back by one.
              --i;
            } else {
              // we got some data from a client
              for (j = 1; j < ufds_.size(); ++j) {
                if (j == i) continue;

                // send to everyone else
//                std::cout << "sending to " << j << " with address = " << ufds_[j].fd << std::endl;
                if (send(ufds_[j].fd, buf_, nbytes_, 0) == -1) {
                  perror("send");
                }
              }
            }
          }
        } // END handle data from client
      } // END got new incoming connection
    } // END looping through file descriptors
  } // END for(;;)--and you thought it would never end!
}