#ifndef __QUEUE_H__

#define __QUEUE_H__

struct node {
    struct node* next;
    int *item;
};

typedef struct node node_t;

void enqueue(int* item);
int* dequeue();

#endif
