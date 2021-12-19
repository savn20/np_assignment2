#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "protocol.h"

using namespace std;

int main(int argc, char *argv[])
{

  /*
    * parses command line input to <ip> <port>
    * terminates program if there's mismatch
  */

  if (argc != 3)
  {
    cerr << "usage: client <ip> <port>\n"
         << "program terminated due to wrong usage" << endl;

    return -1;
  }

  char *serverIp = argv[1];
  char *serverPort = argv[2];
  int socketConnection = -1;
  int response = -1;

  socklen_t serverAddressLen = sizeof(struct sockaddr_in);

  /*
    addrinfo helps to identify host info to bind socket
  */
  addrinfo addressInfo, *serverAddress;
  memset(&addressInfo, 0, sizeof addressInfo);

  addressInfo.ai_family = AF_INET;
  addressInfo.ai_socktype = SOCK_DGRAM;
  addressInfo.ai_flags = AI_PASSIVE;

  if ((response = getaddrinfo(serverIp, serverPort, &addressInfo, &serverAddress)) != 0)
  {
    cerr << "error: unable to connect to specified host\n"
         << "program terminated due to host error" << endl;

    return -2;
  }

  // creating UDP socket with specified ip and port
  if ((socketConnection = socket(serverAddress->ai_family, serverAddress->ai_socktype, serverAddress->ai_protocol)) == -1)
  {
    cerr << "error: failed to create socket\n"
         << "program terminated while creating socket" << endl;

    return -3;
  }

  // connecting to server
  if (connect(socketConnection, serverAddress->ai_addr, serverAddress->ai_addrlen) == -1)
  {
    close(socketConnection);
    cerr << "error: failed to connect server\n"
         << "program terminated due to server connection failure" << endl;

    return -4;
  }

  /*
    * now that we established connection, 
    * we dont need to specify server address in
    * sendto, recvfrom...
    * so removing `serverAddress`
  */
  freeaddrinfo(serverAddress);

  /*
    Initial message for handshake
    type: 22
    message: 0
    protocol: 17
    major_version: 1
    minor_version: 0
  */
  calcMessage clientMessage;
  memset(&clientMessage, 0, sizeof clientMessage);

  calcProtocol serverResponse;
  memset(&serverResponse, 0, sizeof serverResponse);

  clientMessage.type = htons(22);
  clientMessage.message = htonl(0);
  clientMessage.protocol = htons(17);
  clientMessage.major_version = htons(1);
  clientMessage.minor_version = htons(0);

  /*
    host to network
    htons() - helper function to convert data to nertwork byte
  */

  if (sendto(socketConnection, &clientMessage, sizeof(calcMessage), 0,
             (struct sockaddr *)NULL, serverAddressLen) < 0)
  {
    cerr << "error: unable to send message via socket\n"
         << "program terminated due to error while communicating with server" << endl;
    return -3;
  }

  if (recvfrom(socketConnection, &serverResponse, sizeof(calcProtocol), 0,
               (struct sockaddr *)NULL, &serverAddressLen) < 0)
  {
    cerr << "error: no bytes receieved from server\n"
         << "program terminated as there is no response from server" << endl;
    return -4;
  }

  printResponse(serverResponse);

  return 0;
}