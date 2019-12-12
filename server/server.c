#include <stdio.h>      /* printf(), fprintf(), perror() */
#include <sys/socket.h> /* socket(), bind(), connect()   */
#include <arpa/inet.h>  /* sockaddr_in, inet_ntoa()      */
#include <stdlib.h>     /* atoi()                        */
#include <string.h>     /* memset()                      */
#include <unistd.h>     /* close()                       */

#define MAXPENDING 5    /* max outstanding connection requests */
#define RCVBUFSIZE 32   /* size of receive buffer                  */


void handleTCPClient(int clientSocket);
void dieWithError(char *errMsg);


int main(int argc, char *argv[]){
    int servSock;
    int clientSock;
    struct sockaddr_in echoServAddr;
    struct sockaddr_in echoClientAddr;
    unsigned short echoServPort;
    unsigned int clientLen;

    if(argc != 2){
        fprintf(stderr, "Usage: %s <Server Port>\n", argv[0]);
        exit(1);
    }
    echoServPort = atoi(argv[1]); // local port

    if((servSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        dieWithError("socket() failed.");
    }

    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    echoServAddr.sin_port = htons(echoServPort);

    if(bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0){
        dieWithError("bind() failed");
    }
    if(listen(servSock, MAXPENDING) < 0){
        dieWithError("listen() failed");
    }

    for(;;){
        clientLen = sizeof(echoClientAddr);
        if((clientSock = accept(servSock, (struct sockaddr *) &echoClientAddr, &clientLen)) < 0){
            dieWithError("accept() failed");
        }
        printf("Handling client %s\n", inet_ntoa(echoClientAddr.sin_addr));
        handleTCPClient(clientSock);
    }
}

void handleTCPClient(int clientSocket){
    char echoBuffer[RCVBUFSIZE];
    int recvMsgSize;
    if((recvMsgSize = recv(clientSocket, echoBuffer, RCVBUFSIZE, 0)) < 0){
        dieWithError("recv() failed.");
    }
    while(recvMsgSize > 0){
        if(send(clientSocket, echoBuffer, recvMsgSize, 0) != recvMsgSize){
            dieWithError("send() failed.");
        }
        if((recvMsgSize = recv(clientSocket, echoBuffer, RCVBUFSIZE, 0)) < 0){
            dieWithError("recv() failed.");
        }
    }
    close(clientSocket);
}

void dieWithError(char *errMsg){
    perror(errMsg);
    exit(1);
}
