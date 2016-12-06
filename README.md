This package contains a few clients and servers that are produced as an exploration of the possibilities of UNIX sockets
for message passing and communication among processes. The goal is to build some working examples of usage of the TCP and
UDP network protocols (for the latter both singlecast and multicast), and of the select and poll POSIX standard C API calls.

The package can be compiled with:

cmake CMakeLists.txt
make

It will produce four executables:
1) Server
2) TCPClient
3) UDPClient
4) MulticastClient


Usage example:
launch the server with:

./Server

Launch one or more telnet clients on different termianl windows and connect to the default server TCP port (9034):

telnet localhost 9034

Launch one or more (in different terminal windows) MulticastClients to listen to the default server UDP port (10024):

./MulticastClient 127.0.0.1 10024

Type some messages in the telnet terminal and see the MulticastClients echo the messages (as they are echoed to them
by the server). Close or open some telnet clients, close or open some more MulticastClients and see how the Server
automatically handles the new connections.


Details about the executables and the operation modes of the server

1) Server:
For instructions on how to run the server run:

Server -h

There are three optional parameters with which it is possible to specify the TCP port (-p), the UDP port (-u) and the
mode (-m). Four modes are possible:
-m 0 : The server listens to incoming TCP connections and echoes the input messages via *UDP multicast*. It uses *poll* to monitor the input messages.
-m 1 : The server listens to incoming TCP connections and echoes the input messages via *UDP singlecast*. It uses *poll* to monitor the input messages.
-m 2 : The server listens to incoming TCP connections and echoes the input messages via *TCP*. It uses *poll* to monitor the input messages.
-m 3 : The server listens to incoming TCP connections and echoes the input messages via *TCP*. It uses *select* to monitor the input messages.

The client executables can be utilized to listen to the server in the various operation modes:
- TCPClient can listen to the TCP modes (-m 2 and -m 3);
- UDPClient can listen to the UDP singlecast mode (-m 1);
- MulticastClient can listen to the UDP multicast mode (-m 0).
Run the clients without any option to get a message explaining the input parameters.
