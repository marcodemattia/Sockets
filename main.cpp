/*
** selectserver.c -- a cheezy multiperson chat server
*/

#include "src/StreamingServerWithSelect.h"
#include "src/StreamingServerWithPoll.h"
#include "src/DatagramServerWithPoll.h"
#include "src/MulticastServerWithPoll.h"
#include <iostream>

std::string readOption(int argc, char* argv[], const std::string & option, const std::string & defaultValue)
{
  if (option == "-h" && std::string(argv[1]) == "-h") return "1";
  for (int i=1; i<argc; ++i) {
    if (std::string(argv[i]) == option) {
      if (i + 1 < argc) {
        return std::string(argv[i + 1]);
      } else {
        std::cerr << option << " option requires an argument." << std::endl;
        throw;
      }
    }
  }
  return defaultValue;
}

int main(int argc, char* argv[]) {
  if (argc > 1) {
    if (readOption(argc, argv, "-h", "0") == "1") {
      std::cout << "Usage:" << std::endl;
      std::cout << "Server -h: prints this message." << std::endl;
      std::cout << "Server -m 0 -p 9034 -u 10024" << std::endl;
      std::cout << "Where -m is the mode [0-4], -p is the TCP port used for incoming connections, and -u is the UDP";
      std::cout << " port for communicating with the UDP clients or for the multicast clients." << std::endl;
      std::cout << "The values shown above are the default values" << std::endl;
      return 0;
    }
  }

  int mode = std::stoi(readOption(argc, argv, "-m", "0"));
  std::string port = readOption(argc, argv, "-p", "9034");
  std::string udp_port = readOption(argc, argv, "-u", "10024");
  if (mode == 0) {
    std::cout << "Launching MulticastServerWithPoll" << std::endl;
    MulticastServerWithPoll server(port.c_str(), udp_port.c_str());
    server.loop();
  }
  else if (mode == 1) {
    std::cout << "Launching UDPServerWithPoll" << std::endl;
    DatagramServerWithPoll server(port.c_str(), udp_port.c_str());
    server.loop();
  }
  else if (mode == 2) {
    std::cout << "Launching TCPServerWithPoll" << std::endl;
    StreamingServerWithPoll server(port.c_str());
    server.loop();
  }
  else if (mode == 3) {
    std::cout << "Launching TCPServerWithSelect" << std::endl;
    StreamingServerWithSelect server(port.c_str());
    server.loop();
  }
  return 0;
}
