#include <stdio.h>          /* printf(), fprintf(), perror()            */
#include <sys/socket.h>     /* socket(), connect(), send(), receive()   */
#include <arpa/inet.h>      /* sockaddr_in, inet_addr()                 */
#include <stdlib.h>         /* atoi()                                   */
#include <string.h>         /* memset()                                 */
#include <unistd.h>         /* close()                                  */

#define RCVBUFSIZE 32       /* size of receive buffer                   */

void dieWithError(char *errMsg);

// invoke with $ echoclient 127.0.0.1 "Hello World"

int main(int argc, char *argv[]){

    int sock;
    struct sockaddr_in echoServAddr;
    unsigned short echoServPort;
    char *servIP;
    char *echoString;
    char echoBuffer[RCVBUFSIZE];
    unsigned int echoStringLen;
    int bytesRcvd, totalBytesReceived;

    if((argc < 3) || (argc > 4)){
        fprintf(stderr, "Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
        exit(1);
    }


    servIP = argv[1];
    echoString = argv[2];

    if(argc == 4){
        echoServPort = atoi(argv[3]);
    } else {
        echoServPort = 7; // typical echo port
    }

    if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        dieWithError("socket() failed.");
    }

    memset(&echoServAddr, 0, sizeof(echoServAddr));
    echoServAddr.sin_family = AF_INET;
    echoServAddr.sin_addr.s_addr = inet_addr(servIP); // convert str IP to 32-bit binary
    echoServAddr.sin_port = htons(echoServPort); // fix byte ordering

    if(connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0){
        dieWithError("connect() failed.");
    }

    echoStringLen = strlen(echoString);

    if(send(sock, echoString, echoStringLen, 0) != echoStringLen){
        dieWithError("send() sent a different number of bytes than expected");
    }

    totalBytesReceived = 0;
    printf("Received: ");
    while(totalBytesReceived < echoStringLen){
        // buffer size-1 for null term
        if((bytesRcvd = recv(sock, echoBuffer, RCVBUFSIZE - 1, 0)) <= 0){
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

void dieWithError(char *errMsg){
    perror(errMsg);
    exit(1);
}
