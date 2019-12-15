#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h> 
#include <pthread.h>

#include "thpool.h"

#define SERVERPORT 5024
#define BUFSIZE 2048
#define SERVER_BACKLOG 25
#define THREAD_POOL_SIZE 4


void handle_client(void* socket);
int  socket_guard(int result, const char* msg);


int main(int argc, char** argv){
    int srvdsc, clidsc;
    struct sockaddr_in srvaddr, cliaddr;
    thpool_t *tm;
    
    // Create server
    socket_guard((srvdsc = socket(AF_INET, SOCK_STREAM, 0)), "Server create failed");
    
    srvaddr.sin_family = AF_INET;
    srvaddr.sin_addr.s_addr = INADDR_ANY;
    srvaddr.sin_port = htons(SERVERPORT);
    socket_guard(bind(srvdsc, (struct sockaddr*)&srvaddr, sizeof(srvaddr)), 
        "Server bind failed\n"
    );

    socket_guard(listen(srvdsc, SERVER_BACKLOG), "Server listen failed");
    printf("Listening on port %d...\n", SERVERPORT);

    tm = thpool_create(THREAD_POOL_SIZE);
    int c = sizeof(struct sockaddr_in);

    while(1){
        clidsc = accept(srvdsc, (struct sockaddr *)&cliaddr, (socklen_t *)&c);
        if(clidsc == -1){
            printf("Problem accepting new connection.\n");
        } 
        else {
            printf("Accepted new connection.\n");
            if(!thpool_add_work(tm, handle_client, (void*)&clidsc)){
                printf("Error creating thread.\n");
                return 1;
            }
        }
    }
    thpool_wait(tm);
    thpool_destroy(tm);
    return 0;
}

void handle_client(void *sockdsc){
    int socket = *(int *)sockdsc;
    int read_size;
    char buffer[BUFSIZE];

    while((read_size = recv(socket, buffer, BUFSIZE, 0), "Client recv failed") > 0){
        printf("Received: %s\n", buffer);
        write(socket, "ACK", 8);
        if(strcmp(buffer, ":q") == 0){
            read_size = 0;
            break;
        }
    }
    if(read_size == 0){
        printf("Client disconnected.\n");
        fflush(stdout);
    } else if(read_size == -1){
        printf("Client could not be reached.\n");
    }
    close(socket);
}

int socket_guard(int result, const char *msg){
    if(result == -1){
        perror(msg);
        exit(1);
    }
    return result;
}
