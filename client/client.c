#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSIZE 1024


int socketGuard(int result, const char* msg);


int main(int argc, char *argv[]){

    int sock;
    struct sockaddr_in serverAddress;
    unsigned short serverPort;
    char *servIP;
    char *echoString;
    char echoBuffer[BUFSIZE];
    unsigned int echoStringLen;
    int bytesRcvd, totalBytesReceived;

    if(argc != 3){
        printf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
        exit(1);
    }
    servIP = argv[1];
    echoString = argv[2];
    serverPort = atoi(argv[3]);

    sock = socketGuard(socket(PF_INET, SOCK_STREAM, IPPROTO_TCP), "Socket create failed.");
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(servIP);
    serverAddress.sin_port = htons(serverPort);
    socketGuard(connect(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)), "Server connect failed.");

    echoStringLen = strlen(echoString);
    if(send(sock, echoString, echoStringLen, 0) != echoStringLen){
        dieWithError("send() sent a different number of bytes than expected");
    }

    totalBytesReceived = 0;
    printf("Received: ");
    while(totalBytesReceived < echoStringLen){
        // buffer size-1 for null term
        if((bytesRcvd = recv(sock, echoBuffer, BUFSIZE - 1, 0)) <= 0){
            dieWithError("recv() failed or connection closed prematurely.");
        }
        totalBytesReceived += bytesRcvd;
        echoBuffer[bytesRcvd] = '\0';
        printf("%s", echoBuffer);
    }
    printf("\n");

    close(sock);
    exit(0);
}

int socketGuard(int result, const char *msg){
    if(result == -1){
        perror(msg);
        exit(1);
    }
    return result;
}
