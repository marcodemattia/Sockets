/*
** selectserver.c -- a cheezy multiperson chat server
*/

#include "src/ServerWithSelect.h"
#include "src/ServerWithPoll.h"

// #define PORT "9034"   // port we're listening on


int main(void)
{
  // ServerWithSelect server("9034");
  ServerWithPoll server("9034");
  server.loop();
  return 0;
}