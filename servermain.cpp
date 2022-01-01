#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "protocol.h"
#include "calcLib.h"

#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>

#define BACKLOG 5
#define DEBUG
#define USER_LIMIT 50
#define MESSAGE_LEN sizeof(calcMessage)
#define PROTOCOL_LEN sizeof(calcProtocol)

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
        cerr << "usage: server <ip>:<port>\n"
             << "program terminated due to wrong usage" << endl;

        exit(-1);
    }

    char seperator[] = ":";
    string serverIp = strtok(argv[1], seperator);
    string destPort = strtok(NULL, seperator);

    int serverPort = atoi(destPort.c_str());

    int serverSocket = -1,
        clientSocket = -1,
        maxSocket = -1,
        interrupt = -1,
        bytes = -1,
        acceptMultipleClients = 1;

    /*************************************/
    /*   setting up server metadata     */
    /***********************************/
    sockaddr_in address, cliaddr;
    address.sin_family = AF_INET;
    address.sin_port = htons(serverPort);
    address.sin_addr.s_addr = inet_addr(serverIp.c_str());

    fd_set socketSet; // stores multiple sockets
    int users[USER_LIMIT];

    clientResponse buffer;

    for (int i = 0; i < USER_LIMIT; i++)
        users[i] = 0;

    /*************************************/
    /*   creating socket connection     */
    /***********************************/
    if ((serverSocket = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        cerr << "server: failed to create socket\n"
             << "program terminated while socket()" << endl;

        exit(-1);
    }

    /*************************************/
    /*  binding server to ip and port   */
    /***********************************/
    if (bind(serverSocket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        close(serverSocket);
        cerr << "error: failed to bind socket\n"
             << "program terminated while bind()" << endl;

        exit(-1);
    }

    printf("server started listening on %s:%d\n", serverIp.c_str(), serverPort);
    socklen_t addrLen = sizeof(address);

    calcProtocol serverResponse;
    memset(&serverResponse, 0, PROTOCOL_LEN);

    calcMessage sendMessage;
    memset(&sendMessage, 0, MESSAGE_LEN);

    while (1)
    {
        /***********************************************/
        /*   setup to monitor all the active clients  */
        /*********************************************/
        FD_ZERO(&socketSet);

        FD_SET(serverSocket, &socketSet);
        maxSocket = serverSocket;

        for (int i = 0; i < USER_LIMIT; i++)
        {
            clientSocket = users[i];

            if (clientSocket > 0)
                FD_SET(clientSocket, &socketSet);

            if (clientSocket > maxSocket)
                maxSocket = clientSocket;
        }

        /********************************/
        /*  listen for client message  */
        /******************************/
        interrupt = select(maxSocket + 1, &socketSet, NULL, NULL, 0);

        if ((interrupt < 0) && (errno != EINTR))
            cerr << "error: failed to listen interrupts" << endl;

        if (FD_ISSET(serverSocket, &socketSet))
        {
            printf("Message from UDP client: \n");
            memset(&buffer, 0, sizeof buffer);
            bytes = recvfrom(serverSocket, &buffer, PROTOCOL_LEN, 0,
                             (struct sockaddr *)&cliaddr, &addrLen);

            if (bytes == MESSAGE_LEN)
            {
                //TODO: if the response is message, send protocol and add client to id
                printMessage(buffer.message);

                serverResponse.id = 10;
                serverResponse.major_version = htons(1);
                serverResponse.type = htons(2);
                serverResponse.arith = htonl(randomTask());

                if (ntohl(serverResponse.arith) > 4)
                {
                    serverResponse.flValue1 = randomFloat();
                    serverResponse.flValue2 = randomFloat();
                }
                else
                {
                    serverResponse.inValue1 = htonl(randomInt());
                    serverResponse.inValue2 = htonl(randomInt());
                }

                sendto(serverSocket, &serverResponse, PROTOCOL_LEN, 0,
                       (struct sockaddr *)&cliaddr, sizeof(cliaddr));
            }

            if (bytes == PROTOCOL_LEN)
            {
                sendMessage.type = htons(2);
                sendMessage.protocol = htons(17);
                sendMessage.message = htonl(0);
                sendMessage.major_version = htons(1);
                sendMessage.minor_version = htons(0);

                printAssignment(buffer.protocol);
                serverResponse = buffer.protocol;
                performAssignment(&serverResponse);

                if (ntohl(serverResponse.arith) > 4)
                {
                    if (serverResponse.flResult == buffer.protocol.flResult)
                    {
                        sendMessage.message = htonl(1);
                    }
                }
                else
                {
                    if (serverResponse.inResult == buffer.protocol.inResult)
                    {
                        sendMessage.message = htonl(1);
                    }
                }

                sendto(serverSocket, &sendMessage, MESSAGE_LEN, 0,
                       (struct sockaddr *)&cliaddr, sizeof(cliaddr));
            }
        }
    }
}