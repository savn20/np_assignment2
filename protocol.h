// #ifdef __GCC_IEC_559
// #pragma message("GCC ICE 559 defined...")

// #else

// #error *** do not use this platform

// #endif

#include <stdint.h>
#include <iostream>
#include <netinet/in.h>

using namespace std;

/*
   Used in both directions; if
   server->client,type should be set to 1,
   client->server type = 2.
 */
struct __attribute__((__packed__)) calcProtocol
{
  uint16_t type;          // What message is this, 1 = server to client, 2 client to server, 3... reserved , conversion needed
  uint16_t major_version; // 1, conversion needed
  uint16_t minor_version; // 0, conversion needed
  uint32_t id;            // Server side identification with operation. Client must return the same ID as it got from Server., conversion needed
  uint32_t arith;         // What operation to perform, see mapping below.
  int32_t inValue1;       // integer value 1, conversion needed
  int32_t inValue2;       // integer value 2, conversion needed
  int32_t inResult;       // integer result, conversion needed
  double flValue1;        // float value 1,NO NEED TO do host to Network or Network to Host conversion here, we are using equivalent platforms
  double flValue2;        // float value 2,NO NEED TO do host to Network or Network to Host conversion here, we are using equivalent platforms
  double flResult;        // float result,NO NEED TO do host to Network or Network to Host conversion here, we are using equivalent platforms
};

struct __attribute__((__packed__)) calcMessage
{
  uint16_t type;    // See below, conversion needed
  uint32_t message; // See below, conversion needed

  // Protocol, UDP = 17, TCP = 6, other values are reserved.
  uint16_t protocol;      // conversion needed
  uint16_t major_version; // 1, conversion needed
  uint16_t minor_version; // 0 , conversion needed
};

union __attribute__((__packed__)) clientResponse
{
  struct calcMessage message;
  struct calcProtocol protocol;
};

struct __attribute__((__packed__)) calcJob
{
  uint32_t id;
  uint32_t arith;
  double flResult;
  int32_t inResult;
  long timestamp;
};

void printMessage(calcMessage message)
{
  cout << "--message--" << endl;
  cout << "type : " << ntohs(message.type) << endl;
  cout << "message : " << ntohl(message.message) << endl;
  cout << "protocol : " << ntohs(message.protocol) << endl;
  cout << "major_version : " << ntohs(message.major_version) << endl;
  cout << "minor_version : " << ntohs(message.minor_version) << endl;
}

void printResponse(calcProtocol protocol, bool fromServer = true)
{
  fromServer ? cout << "--server response--" << endl : cout << "--client response--" << endl;
  cout << "type: " << ntohs(protocol.type) << endl;
  cout << "major_version: " << ntohs(protocol.major_version) << endl;
  cout << "minor_version: " << ntohs(protocol.minor_version) << endl;
  cout << "id: " << protocol.id << endl;
  cout << "arith: " << ntohl(protocol.arith) << endl;
  cout << "inValue1: " << ntohl(protocol.inValue1) << endl;
  cout << "inValue2: " << ntohl(protocol.inValue2) << endl;
  cout << "inResult: " << ntohl(protocol.inResult) << endl;
  cout << "flValue1: " << protocol.flValue1 << endl;
  cout << "flValue2: " << protocol.flValue2 << endl;
  cout << "flResult: " << protocol.flResult << endl;
}

/* arith mapping in calcProtocol
1 - add
2 - sub
3 - mul
4 - div
5 - fadd
6 - fsub
7 - fmul
8 - fdiv

other numbers are reserved

*/

/*
   calcMessage.type
   1 - server-to-client, text protocol
   2 - server-to-client, binary protocol
   3 - server-to-client, N/A
   21 - client-to-server, text protocol
   22 - client-to-server, binary protocol
   23 - client-to-serve, N/A

   calcMessage.message

   0 = Not applicable/availible (N/A or NA)
   1 = OK   // Accept
   2 = NOT OK  // Reject

*/

void _printOperation(int type)
{
  switch (type)
  {
  case 1:
  case 5:
    cout << "add";
    break;
  case 2:
  case 6:
    cout << "sub";
    break;
  case 3:
  case 7:
    cout << "mul";
    break;
  case 4:
  case 8:
    cout << "div";
    break;
  default:
    break;
  }
}

void printAssignment(calcProtocol response)
{
  int operation = ntohl(response.arith);

  if (operation > 4)
  {
    cout << "server: f";
    _printOperation(operation);
    cout << " " << response.flValue1 << " " << response.flValue2 << endl;
  }
  else
  {
    cout << "server: ";
    _printOperation(operation);
    cout << " " << ntohl(response.inValue1) << " " << ntohl(response.inValue2) << endl;
  }
}

void performAssignment(calcProtocol *response, bool fromServer = false)
{
  int operation = ntohl(response->arith);
  int inResult = 0;
  double flResult = 0.0;

  switch (operation)
  {
  case 1:
    inResult = ntohl(response->inValue1) + ntohl(response->inValue2);
    response->inResult = htonl(inResult);
    break;
  case 2:
    inResult = ntohl(response->inValue1) - ntohl(response->inValue2);
    response->inResult = htonl(inResult);
    break;
  case 3:
    inResult = ntohl(response->inValue1) * ntohl(response->inValue2);
    response->inResult = htonl(inResult);
    break;
  case 4:
    inResult = ntohl(response->inValue1) / ntohl(response->inValue2);
    response->inResult = htonl(inResult);
    break;
  case 5:
    flResult = response->flValue1 + response->flValue2;
    response->flResult = flResult;
    break;
  case 6:
    flResult = response->flValue1 - response->flValue2;
    response->flResult = flResult;
    break;
  case 7:
    flResult = response->flValue1 * response->flValue2;
    response->flResult = flResult;
    break;
  case 8:
    flResult = response->flValue1 / response->flValue2;
    response->flResult = flResult;
    break;

  default:
    break;
  }

  if (operation > 4)
  {
    fromServer ? cout << "server: calculated " : cout << "client: sent result ";
    cout << flResult << endl;
  }
  else
  {
    fromServer ? cout << "server: calculated " : cout << "client: sent result ";
    cout << inResult << endl;
  }
}