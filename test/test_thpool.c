#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#include "../server/thpool.h"

#define THREADS 4
#define WORKSIZE 100


void worker(void *arg){
    int *val = arg;
    int old = *val;
    *val += 1000;
    printf("old=%d, val=%d\n", old, *val);
}

int main(){
    thpool_t *tm;
    int *vals;
    
    tm = thpool_create(THREADS);
    vals = calloc(WORKSIZE, sizeof(*vals));

    for(int i = 0; i < WORKSIZE; i++){
        vals[i] = i;
        thpool_add_work(tm, worker, vals+i);
    }
    thpool_wait(tm);
    for(int i = 0; i < WORKSIZE; i++){
        printf("%d\n", vals[i]);
    }

    free(vals);
    thpool_destroy(tm);
    return 0;
}
