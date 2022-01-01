#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "protocol.h"

#define MESSAGE_LEN sizeof(calcMessage)
#define PROTOCOL_LEN sizeof(calcProtocol)

// comment the DEBUG macro to turn off comments in the console
#define DEBUG

using namespace std;

int main(int argc, char *argv[])
{
  // disables debugging when there's no DEBUG macro defined
#ifndef DEBUG
  cout.setstate(ios_base::failbit);
  cerr.setstate(ios_base::failbit);
#endif

  /*************************************/
  /*  getting ip and port from args   */
  /***********************************/
  if (argc != 2)
  {
    cerr << "usage: client <ip>:<port>\n"
         << "program terminated due to wrong usage" << endl;

    return -1;
  }

  char delim[] = ":";
  char *serverIp = strtok(argv[1], delim);
  char *serverPort = strtok(NULL, delim);

  int socketConnection = -1;
  int responseBytes = -1;
  int wait = 3;

  socklen_t serverAddressLen = sizeof(struct sockaddr_in);
  timeval timeout;
  timeout.tv_sec = 5;

  /*************************************/
  /*   setting up server metadata     */
  /***********************************/
  addrinfo addressInfo, *serverAddress;
  memset(&addressInfo, 0, sizeof addressInfo);

  addressInfo.ai_family = AF_INET;
  addressInfo.ai_socktype = SOCK_DGRAM;
  addressInfo.ai_flags = AI_PASSIVE;

  if ((responseBytes = getaddrinfo(serverIp, serverPort, &addressInfo, &serverAddress)) != 0)
  {
    cerr << "error: unable to connect to specified host\n"
         << "program terminated due to host error" << endl;

    return -2;
  }

  cout << "client: establishing connection to " << serverIp << ":" << serverPort << endl;

  // creating UDP socket with specified ip and port
  if ((socketConnection = socket(serverAddress->ai_family, serverAddress->ai_socktype, serverAddress->ai_protocol)) == -1)
  {
    cerr << "error: failed to create socket\n"
         << "program terminated while creating socket" << endl;

    return -3;
  }

  // sets the timeout aka. time to wait until the response is received
  setsockopt(socketConnection, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
  setsockopt(socketConnection, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof timeout);

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
  memset(&clientMessage, 0, MESSAGE_LEN);

  calcProtocol serverResponse;
  memset(&serverResponse, 0, PROTOCOL_LEN);

  clientMessage.type = htons(22);
  clientMessage.message = htonl(0);
  clientMessage.protocol = htons(17);
  clientMessage.major_version = htons(1);
  clientMessage.minor_version = htons(0);

  wait = 3;
  responseBytes = -1;

  /*************************************/
  /*  handshake: sending version 1.0  */
  /***********************************/
  cout << "handshake: sending version 1.0 to " << serverIp << ":" << serverPort << "..." << endl;

  while (wait)
  {
    if (wait < 3)
      cout << "client: retransmitting message..." << endl;

    if (sendto(socketConnection, &clientMessage, MESSAGE_LEN, 0,
               (struct sockaddr *)NULL, serverAddressLen) < 0)
    {
      cerr << "error: unable to send message via socket\n"
           << "program terminated due to error while communicating with server" << endl;
      return -3;
    }

    responseBytes = recvfrom(socketConnection, &serverResponse, PROTOCOL_LEN, 0,
                             (struct sockaddr *)NULL, &serverAddressLen);

    if (responseBytes != -1)
      break;

    wait--;
  }

  if (responseBytes == -1)
  {
    printf("server did not reply\n");
    cerr << "error: no bytes receieved from server\n"
         << "program terminated as there is no response from server" << endl;
    return -4;
  }
  else if (responseBytes == MESSAGE_LEN)
  {
    cerr << "server: NOT OK!" << endl
         << "error: server doesn't support the version provided" << clientMessage.major_version << endl
         << "program terminated due to version error" << endl;
    return -5;
  }
  else
  {
    printf("connected to server %s:%s\n", serverIp, serverPort);
    printAssignment(serverResponse);
  }

  performAssignment(&serverResponse);

  /****************************************/
  /* task: sending result of given task  */
  /**************************************/
  wait = 3;
  responseBytes = -1;

  while (wait)
  {
    if (wait < 3)
      cout << "client: retransmitting message..." << endl;

    if (sendto(socketConnection, &serverResponse, PROTOCOL_LEN, 0,
               (struct sockaddr *)NULL, serverAddressLen) < 0)
    {
      cerr << "error: unable to send message via socket\n"
           << "program terminated due to error while communicating with server" << endl;
      return -3;
    }

    responseBytes = recvfrom(socketConnection, &clientMessage, MESSAGE_LEN, 0,
                             (struct sockaddr *)NULL, &serverAddressLen);

    if (responseBytes != -1)
      break;

    wait--;
  }

  if (responseBytes == -1)
  {
    printf("server did not reply\n");
    cerr << "error: no bytes receieved from server\n"
         << "program terminated as there is no response from server" << endl;
    return -4;
  }

  // server response

  ntohl(clientMessage.message) == 1 ? cout << "server: OK!\n" : cerr << "server: NOT OK!\n";

  return 0;
}