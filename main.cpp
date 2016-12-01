/*
** selectserver.c -- a cheezy multiperson chat server
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT "9034"   // port we're listening on

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
  fd_set master;    // master file descriptor list
  fd_set read_fds;  // temp file descriptor list for select()
  int fdmax;        // maximum file descriptor number

  int listener;     // listening socket descriptor
  int newfd;        // newly accept()ed socket descriptor
  struct sockaddr_storage remoteaddr; // client address
  socklen_t addrlen;

  char buf[256];    // buffer for client data
  int nbytes;

  char remoteIP[INET6_ADDRSTRLEN];

  int yes=1;        // for setsockopt() SO_REUSEADDR, below
  int i, j, rv;

  struct addrinfo hints, *ai, *p;

  // The FD_ZERO macro initializes the file descriptor set to have zero bits for all file descriptors.
  FD_ZERO(&master);    // clear the master and temp sets
  FD_ZERO(&read_fds);

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
  for(p = ai; p != NULL; p = p->ai_next) {
    listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
    if (listener < 0) {
      continue;
    }

    // lose the pesky "address already in use" error message
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
      close(listener);
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
  if (listen(listener, 10) == -1) {
    perror("listen");
    exit(3);
  }

  // add the listener to the master set
  FD_SET(listener, &master);

  // keep track of the biggest file descriptor
  fdmax = listener; // so far, it's this one

  // main loop
  for(;;) {
    read_fds = master; // copy it
    // Ignoring write and exceptions (third and fourth field) and no time limit (fifth field).
    // This function will block until some socket becomes active (read bit is changed). It only
    // tells you something has changed, you still need to go through the full list of read_fds
    // to see which ones have changed.
    if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
      perror("select");
      exit(4);
    }

    // run through the existing connections looking for data to read
    for(i = 0; i <= fdmax; i++) {
      // The FD_ISSET macro allows to check if the socket has changed.
      if (FD_ISSET(i, &read_fds)) { // we got one!!
        // We treat the listener in a special way. We accept connections on the listener socket.
        // If we find a new connection on this socket we add it to the master list of fds.
        if (i == listener) {
          // handle new connections
          addrlen = sizeof remoteaddr;
          newfd = accept(listener,
                         (struct sockaddr *)&remoteaddr,
                         &addrlen);

          if (newfd == -1) {
            perror("accept");
          } else {
            FD_SET(newfd, &master); // add to master set
            if (newfd > fdmax) {    // keep track of the max
              fdmax = newfd;
            }
            printf("selectserver: new connection from %s on "
                       "socket %d\n",
                   inet_ntop(remoteaddr.ss_family,
                             get_in_addr((struct sockaddr*)&remoteaddr),
                             remoteIP, INET6_ADDRSTRLEN),
                   newfd);
          }
        } else {
          // handle data from a client. It accepts up to 256 chars (the size of buf) and we set no flags (last field).
          if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
            // got error or connection closed by client
            if (nbytes == 0) {
              // connection closed
              printf("selectserver: socket %d hung up\n", i);
            } else {
              perror("recv");
            }
            close(i); // bye!
            FD_CLR(i, &master); // remove from master set
          } else {
            // we got some data from a client
            for(j = 0; j <= fdmax; j++) {
              // send to everyone!
              if (FD_ISSET(j, &master)) {
                // except the listener and ourselves
                if (j != listener && j != i) {
                  if (send(j, buf, nbytes, 0) == -1) {
                    perror("send");
                  }
                }
              }
            }
          }
        } // END handle data from client
      } // END got new incoming connection
    } // END looping through file descriptors
  } // END for(;;)--and you thought it would never end!

  return 0;
}