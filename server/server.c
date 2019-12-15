#include <stdio.h>      /* printf(), fprintf(), perror() */
#include <sys/socket.h> /* socket(), bind(), connect()   */
#include <arpa/inet.h>  /* sockaddr_in, inet_ntoa()      */
#include <stdlib.h>     /* atoi()                        */
#include <string.h>     /* memset()                      */
#include <unistd.h>     /* close()                       */
#include <pthread.h>

#include "queue.h"

#define SERVERPORT 5024
#define BUFSIZE 4096
#define SERVER_BACKLOG 100
#define THREAD_POOL_SIZE 20


void* handleClient(void* socket);
void* threadWrapper(void *arg);
void  enqueueClient(int socket);
int   socketGuard(int result, const char* msg);


int activeClients = 0;
pthread_t thread_pool[THREAD_POOL_SIZE];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condSig = PTHREAD_COND_INITIALIZER;


int main(int argc, char** argv){
    int serverSocket, clientSocket, addrSize;
    struct sockaddr_in serverAddr, clientAddr;
    
    // Create server
    socketGuard((serverSocket = socket(AF_INET, SOCK_STREAM, 0)), "Server create failed.");
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SERVERPORT);
    socketGuard(bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)), 
        "Server bind failed."
    );
    socketGuard(listen(serverSocket, SERVER_BACKLOG), "Server listen failed.");

    // Create thread pool
    for(int i = 0; i < THREAD_POOL_SIZE; i++){
        pthread_create(&thread_pool[i], NULL, threadWrapper, NULL);
    }
    while(1){
        addrSize = sizeof(struct sockaddr);
        socketGuard(clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, (socklen_t*) &addrSize), 
            "Client accept failed."
        );
        enqueueClient(clientSocket);
    }
    return 0;
}

void enqueueClient(int socket){
    int *pclient = malloc(sizeof(int));
    *pclient = socket;

    pthread_mutex_lock(&mutex);
    enqueue(pclient);
    printf("Client enqueued. %d client(s) active.\n", ++activeClients);
    pthread_cond_signal(&condSig);
    pthread_mutex_unlock(&mutex);
}

void* threadWrapper(void *arg){
    while(1){
        pthread_mutex_lock(&mutex);
        int *client;
        if((client = dequeue()) == NULL){
            pthread_cond_wait(&condSig, &mutex);
            client = dequeue();
            printf("Client dequeued. %d client(s) active.\n", --activeClients);
        }
        pthread_mutex_unlock(&mutex);
        if(client != NULL){
            handleClient(client);
        }
    }
}

void *handleClient(void* socketDesc){
    /*int socket = *(int*) socketDesc;
    char inBuffer[BUFSIZE], outBuffer[BUFSIZE];
    int recvMsgSize = 0;

    free(socketDesc);
    
    while((socketGuard(recv(socket, inBuffer, BUFSIZE, 0), "Client recv failed.")) > 0){
        strncpy(outBuffer, "Hello client!", BUFSIZE);
        socketGuard(send(socket, outBuffer, recvMsgSize, 0), "Client send failed.");
    }
    close(socket);
    return NULL;*/

    int socket = *(int*)socketDesc;
    free(socketDesc);
    int read_size;
    char* message, client_message[2000];

    message = "Greetings, I am your connection handler\n";
    socketGuard(write(socket, message, strlen(message)), "Client write failed.");

    message = "Now type something and i shall repeat what you type \n";
    socketGuard(write(socket , message , strlen(message)), "Client write failed.");

    while((read_size = socketGuard(recv(socket, client_message, 2000, 0), "Client recv failed.")) > 0){
        message = "You wrote: ";
        socketGuard(write(socket, message, strlen(message)), "Client write failed.");
        socketGuard(write(socket, client_message, read_size), "Client write failed.");
    }
    if(read_size == 0){
        printf("Client disconnected.\n");
        fflush(stdout);
    }
    close(socket);
    return NULL;
}

int socketGuard(int result, const char *msg){
    if(result == -1){
        perror(msg);
        exit(1);
    }
    return result;
}
