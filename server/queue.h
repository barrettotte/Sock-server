#ifndef QUEUE_H_

#define QUEUE_H_

struct node {
    struct node* next;
    int *item;
};

typedef struct node node_t;

void enqueue(int* item);
int* dequeue();

#endif
