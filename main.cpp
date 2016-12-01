/*
** selectserver.c -- a cheezy multiperson chat server
*/

#include "src/StreamingServerWithSelect.h"
#include "src/StreamingServerWithPoll.h"
#include "src/DatagramServerWithPoll.h"

 #define PORT "9034"   // port we're listening on


int main(void)
{
  // StreamingServerWithSelect server("9034");
  // StreamingServerWithPoll server("9034");
  DatagramServerWithPoll server("9034", "10024");
  server.loop();
  return 0;
}
