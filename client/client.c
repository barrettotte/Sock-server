#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5024
#define BUFSIZE 2048


int socket_guard(int result, const char *msg);


int main(int argc, char *argv[]){
    int sock;
    struct sockaddr_in servaddr;
    char buffer[BUFSIZE];
    char *server = "sock-kvdb";
    char *username;

    if(argc <= 1){
        username = "anon";
    } else {
        username = argv[1];
    }

    sock = socket_guard(socket(AF_INET, SOCK_STREAM, 0), 
        "Error creating client socket");
    printf("Socket created.\n");

    memset(&servaddr, '\0', sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    socket_guard(connect(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)),
        "Error connecting to server");
    printf("Established connection to server.\n\n");

    while(1){
        printf("%s@%s ~> ", username, server);
        scanf("%s", &buffer[0]);
        socket_guard(send(sock, buffer, BUFSIZE, 0), "Error sending data");

        if(strcmp(buffer, ":q") == 0){
            printf("Disconnected from server.\n");
            close(sock);
            exit(1);
        }
        memset(buffer, 0, BUFSIZE);

        socket_guard(recv(sock, buffer, BUFSIZE, 0), "Error receiving data");
        printf("Response:  %s\n", buffer);
    }
    return 0;
}

int socket_guard(int result, const char *msg){
    if(result == -1){
        perror(msg);
        exit(1);
    }
    return result;
}
