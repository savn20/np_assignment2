#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <bits/stdc++.h>
#include "protocol.h"
#include "calcLib.c"

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

vector<calcJob> jobs(USER_LIMIT);
int loopCount = 0;

void addJob(calcJob job)
{
    cout << "adding new job: " << job.id << endl;

    jobs.push_back(job);
}

calcJob *getJobById(int id)
{
    vector<calcJob>::iterator it = jobs.begin();

    for (; it != jobs.end(); ++it)
        if (id == it->id)
            break;

    return it.base();
}

void removeJob(int jobId)
{
    cout << "removing job: " << jobId << endl;

    vector<calcJob>::iterator itr = jobs.begin();

    for (; itr != jobs.end(); ++itr)
        if (jobId == itr->id)
            break;

    if (itr->id == jobId)
    {
        itr->id = -1;
        itr->timestamp = 0;
    }
}

void checkJobbList(int signum)
{
    time_t currentTimestamp = time(nullptr);
    cout << "removing jobs that were created before 10s" << endl;

    // removing jobs that were created before 10s
    vector<calcJob>::iterator itr = jobs.begin();
    for (; itr != jobs.end(); ++itr)
    {
        if (itr->timestamp == 0)
            continue;

        if ((currentTimestamp - itr->timestamp) >= 10)
        {
            cout << "removing job " << itr->id << endl;
            itr->id = -1;
            itr->timestamp = 0;
        }
    }
}

int main(int argc, char *argv[])
{
    // disables debugging when there's no DEBUG macro defined
#ifndef DEBUG
    cout.setstate(ios_base::failbit);
    cerr.setstate(ios_base::failbit);
#endif

    /* 
     Prepare to setup a reoccurring event every 10s. 
     If it_interval, or it_value is omitted, 
        it will be a single alarm 10s after it has been set. 
    */
    struct itimerval alarmTime;

    /* Regiter a callback function, associated with the SIGALRM signal, which will be raised when the alarm goes of */
    signal(SIGALRM, checkJobbList);

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

    calcProtocol serverJob;
    memset(&serverJob, 0, PROTOCOL_LEN);

    calcMessage serverResponse;
    memset(&serverResponse, 0, MESSAGE_LEN);

    while (1)
    {
        sleep(1);
        loopCount++;
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
            memset(&buffer, 0, sizeof buffer);
            bytes = recvfrom(serverSocket, &buffer, PROTOCOL_LEN, 0,
                             (struct sockaddr *)&cliaddr, &addrLen);

            /*********************************/
            /* accept client and assign job */
            /*******************************/
            if (bytes == MESSAGE_LEN)
            {
                /* If the server does not support the protocol provides by the client, 
                // it responds with 'calcMessage', 
                    type=2,
                    message=2, 
                    major_version=1,
                    minor_version=0.
                */
                if (ntohs(buffer.message.type) != 22 ||
                    ntohs(buffer.message.protocol) != 17 ||
                    ntohs(buffer.message.major_version) != 1)
                {
                    serverResponse.type = htons(2);
                    serverResponse.message = htonl(2);
                    serverResponse.major_version = htons(1);
                    serverResponse.minor_version = htons(0);

                    sendto(serverSocket, &serverResponse, MESSAGE_LEN, 0,
                           (struct sockaddr *)&cliaddr, sizeof(cliaddr));

                    continue;
                }

                serverJob.id = (uint32_t)randomInt();
                serverJob.major_version = htons(1);
                serverJob.type = htons(2);
                serverJob.arith = htonl(randomTask());

                if (ntohl(serverJob.arith) > 4)
                {
                    serverJob.flValue1 = randomFloat();
                    serverJob.flValue2 = randomFloat();
                }
                else
                {
                    serverJob.inValue1 = htonl(randomInt());
                    serverJob.inValue2 = htonl(randomInt());
                }

                performAssignment(&serverJob);

                time_t result = time(nullptr);
                calcJob *newJob = new calcJob();

                newJob->id = serverJob.id;
                newJob->arith = serverJob.arith;
                newJob->inResult = serverJob.inResult;
                newJob->flResult = serverJob.flResult;
                newJob->timestamp = result;
                serverJob.flResult = 0.0;
                serverJob.inResult = 0;

                addJob(*newJob);
                alarmTime.it_interval.tv_sec = 0;
                alarmTime.it_interval.tv_usec = 0;
                alarmTime.it_value.tv_sec = 10;
                alarmTime.it_value.tv_usec = 0;
                setitimer(ITIMER_REAL, &alarmTime, NULL); // Start/register the alarm.

                free(newJob);

                sendto(serverSocket, &serverJob, PROTOCOL_LEN, 0,
                       (struct sockaddr *)&cliaddr, sizeof(cliaddr));
            }

            /***********************************/
            /*  verify the result client sent */
            /*********************************/
            if (bytes == PROTOCOL_LEN)
            {
                serverResponse.type = htons(2);
                serverResponse.protocol = htons(17);
                serverResponse.message = htonl(0);
                serverResponse.major_version = htons(1);
                serverResponse.minor_version = htons(0);

                serverJob = buffer.protocol;
                printAssignment(serverJob);

                calcJob *givenJob = new calcJob();
                givenJob = getJobById(buffer.protocol.id);

                // compare the result
                if (ntohl(serverJob.arith) > 4)
                {
                    if (serverJob.flResult == givenJob->flResult)
                    {
                        serverResponse.message = htonl(1);
                    }
                }
                else
                {
                    if (serverJob.inResult == givenJob->inResult)
                    {
                        serverResponse.message = htonl(1);
                    }
                }

                time_t currentTimestamp = time(nullptr);

                // compare the id
                if (serverJob.id != givenJob->id || (currentTimestamp - givenJob->timestamp) >= 10)
                {
                    serverResponse.message = htonl(0);
                }

                // manually removing the job after it's done
                removeJob(serverJob.id);

                sendto(serverSocket, &serverResponse, MESSAGE_LEN, 0,
                       (struct sockaddr *)&cliaddr, sizeof(cliaddr));
            }
        }
    }
}