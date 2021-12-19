#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
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
  int serverPort = atoi(argv[2]);
  int socketConnection = -1;

  sockaddr_in serverAddress;
  int serverAddLen = sizeof(serverAddress);

  cout << "Establishing connection to " << serverIp << " " << serverPort << endl;

  // creating UDP socket with specified ip and port
  socketConnection = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (socketConnection < 0)
  {
    cerr << "error: failed to create socket\n"
         << "program terminated while creating socket" << endl;

    return -2;
  }

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_port = htons(serverPort);
  serverAddress.sin_addr.s_addr = inet_addr(serverIp);

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

  int responseCode = sendto(socketConnection, &clientMessage, sizeof(calcMessage), 0,
                            (struct sockaddr *)&serverAddress, sizeof(serverAddress));

  if (responseCode < 0)
  {
    cerr << "error: unable to send message via socket\n"
         << "program terminated due to error code: " << responseCode << endl;
    return -3;
  }

  int responseBytes = recvfrom(socketConnection, &serverResponse, sizeof(calcProtocol), 0,
                               (struct sockaddr *)&serverAddress, (socklen_t *)&serverAddress);

  if (responseBytes < 0)
  {
    cerr << "error: no bytes receieved from server\n"
         << "program terminated as there is no response from server" << endl;
    return -4;
  }

  printResponse(serverResponse);

  return 0;
}