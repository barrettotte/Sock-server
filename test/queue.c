#include "queue.h"
#include <stdlib.h>


node_t* head = NULL;
node_t* tail = NULL;


void enqueue(int* item){
    node_t* newNode = malloc(sizeof(node_t));
    newNode->item = item;
    newNode->next = NULL;
    
    if(tail == NULL){
        head = newNode;
    } else {
        tail->next = newNode;
    }
    tail = newNode;
}

int* dequeue(){
    if(head == NULL){
        return NULL;
    } else {
        int* item = head->item;
        node_t *tmp = head;
        head = head->next;

        if(head == NULL){
            tail = NULL;
        }
        free(tmp);
        return item;
    }
}
