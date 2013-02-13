#include <iostream>
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), bind(), and connect() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

using namespace std;

const unsigned short PORT = 1130;       /* Port for server */
#define MAXPENDING 5                    /* Maximum outstanding connection requests */
#define RCVBUFSIZE 32                   /* Size of receive buffer */

void DieWithError(char *errorMessage);  /* Error handling function */
void HandleTCPClient(int clntSocket);   /* TCP client handling function */

int main()
{
    int servSock;                       /* Socket descriptor for Server*/
    int clntSock;                       /* Socket descriptor for client */
    struct sockaddr_in servAddr;        /* Local Address */
    struct sockaddr_in clntAddr;        /* Client address */
    unsigned int clntLen;               /* Length of client address data structure */
    cout << "Starting server..." << endl;

    if((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        DieWithError("socket() failed...");

    /* Construct local address structure */
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servAddr.sin_port = htons(PORT);

    /* Bind to the local address */
    if(bind(servSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        DieWithError("bind() failed...");

    /* Mark the socket so it will listen for incoming connections */
    if(listen(servSock, MAXPENDING) < 0)
        DieWithError("listen() failed...");

    cout << "Waiting for requests..." << endl;
	
    while(1) /* Run forever */
    {
        /* Set the size of the in-out parameter */
        clntLen = sizeof(clntAddr);

        /* Wait for a client to connect */
        if ((clntSock = accept(servSock, (struct sockaddr *) &clntAddr,
                               &clntLen)) < 0)
            DieWithError("accept() failed");

        /* clntSock is connected to a client! */

        printf("Handling client %s\n", inet_ntoa(clntAddr.sin_addr));

        HandleTCPClient(clntSock);
    }

    return 0;
}

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

void HandleTCPClient(int clntSocket)
{
    char buffer[RCVBUFSIZE];        /* Buffer for echo string */
bzero(buffer, 0);
    int recvMsgSize;                    /* Size of received message */
	recvMsgSize = 4;
    /* Receive message from client */
//    if ((recvMsgSize = recv(clntSocket, buffer, RCVBUFSIZE, 0)) < 0)
//        DieWithError("recv() failed");
    /* Send received string and receive again until end of transmission */
    while (recvMsgSize > 0)      /* zero indicates end of transmission */
//char a = 'a';
 //   while(1)
{
	cout << "handling" << endl;
	char s[10];
	cin >> s;
        /* Echo message back to client */
        if (send(clntSocket, s, sizeof(s), 0) != sizeof(s))
            DieWithError("send() failed");
        /* See if there is more data to receive */
  //      if ((recvMsgSize = recv(clntSocket, s, sizeOf(s), 0)) < 0)
  //          DieWithError("recv() failed");
	cout << recvMsgSize << endl;
	cout << buffer << endl;
    }
	cout << "closing";

    close(clntSocket);    /* Close client socket */
	cout <<endl;
}
