#include <stdlib.h>
#include <assert.h>
#include <pthread.h>
#include "thpool.h"


typedef struct thpool_work{
    thread_func_t       func;
    void               *arg;
    struct thpool_work *next;
} thpool_work_t;

struct thpool{
    thpool_work_t   *work_head;
    thpool_work_t   *work_tail;
    pthread_mutex_t  work_lock;     /* lock used by all threads, synchronize work fetching */
    pthread_cond_t   notify;         /* signal when there is work to be processed */
    pthread_cond_t   done;   /* signal when there are no threads processing */
    int              active_threads; /* active threads */
    int              alive_threads;  /* alive threads */
    bool             halt;           /* terminate pool */
};


static thpool_work_t *thpool_work_create(thread_func_t func, void *arg){
    assert(func);
    thpool_work_t *work;
    work = malloc(sizeof(*work));
    work->func = func;
    work->arg = arg;
    work->next = NULL;
    return work;
}

static void thpool_work_destroy(thpool_work_t *work){
    assert(work);
    free(work);
}

static thpool_work_t *thpool_work_get(thpool_t *tp){
    assert(tp);
    thpool_work_t *work;
    work = tp->work_head;
    if(work == NULL){
        return NULL;
    }
    if(work->next == NULL){
        tp->work_head = NULL;
        tp->work_tail = NULL;
    } else {
        tp->work_head = work->next;
    }
    return work;
}

static void *thpool_worker(void *arg){
    thpool_t *tp = arg;
    thpool_work_t *work;
    while(1){
        pthread_mutex_lock(&(tp->work_lock));
        if(tp->halt){
            break;
        }
        /* check for available work, wait around if none */
        if(tp->work_head == NULL){
            pthread_cond_wait(&(tp->notify), &(tp->work_lock));
        }

        /* perform work */
        work = thpool_work_get(tp);
        tp->active_threads++;
        pthread_mutex_unlock(&(tp->work_lock));
        if(work != NULL){
            work->func(work->arg);
            thpool_work_destroy(work);
        }

        /* finish work */
        pthread_mutex_lock(&(tp->work_lock));
        tp->active_threads--;
        if(!tp->halt && tp->active_threads == 0 && tp->work_head == NULL){
            pthread_cond_signal(&(tp->done));
        }
        pthread_mutex_unlock(&(tp->work_lock));
    }
    /* kill thread */
    tp->alive_threads--;
    pthread_cond_signal(&(tp->done));
    pthread_mutex_unlock(&(tp->work_lock));
    return NULL;
}

thpool_t *thpool_create(int threadnum){
    thpool_t *tm;
    pthread_t thread;

    tm = calloc(1, sizeof(*tm));
    tm->alive_threads = threadnum;
    pthread_mutex_init(&(tm->work_lock), NULL);
    pthread_cond_init(&(tm->notify), NULL);
    pthread_cond_init(&(tm->done), NULL);
    tm->work_head = NULL;
    tm->work_tail = NULL;

    /* create threads for pool */
    for(int i = 0; i < threadnum; i++){
        pthread_create(&thread, NULL, thpool_worker, tm);
        pthread_detach(thread);
    }
    return tm;
}

void thpool_destroy(thpool_t *tp){
    assert(tp);
    thpool_work_t *work;
    thpool_work_t *worktmp;
    
    /* finish active processes */
    pthread_mutex_lock(&(tp->work_lock));
    work = tp->work_head;
    while(work != NULL){
        worktmp = work->next;
        thpool_work_destroy(work);
        work = worktmp;
    }
    tp->halt = 1;
    pthread_cond_broadcast(&(tp->notify));
    pthread_mutex_unlock(&(tp->work_lock));
    
    thpool_wait(tp); /* wait in case threads are still processing */
    pthread_mutex_destroy(&(tp->work_lock));
    pthread_cond_destroy(&(tp->notify));
    pthread_cond_destroy(&(tp->done));
    free(tp);
}

bool thpool_add_work(thpool_t *tp, thread_func_t func, void *arg){
    thpool_work_t *work;
    if(tp == NULL){
        return false;
    }
    work = thpool_work_create(func, arg);
    if(work == NULL){
        return false;
    }

    /* enqueue new work */
    pthread_mutex_lock(&(tp->work_lock));
    if(tp->work_head == NULL){
        tp->work_head = work;
        tp->work_tail = tp->work_head;
    } else{
        tp->work_tail->next = work;
        tp->work_tail = work;
    }
    pthread_cond_broadcast(&(tp->notify));
    pthread_mutex_unlock(&(tp->work_lock));
    return true;
}

void thpool_wait(thpool_t *tp){
    assert(tp);
    pthread_mutex_lock(&(tp->work_lock));
    while(1){
        if((!tp->halt && tp->active_threads != 0) || (tp->halt && tp->alive_threads != 0)){
            pthread_cond_wait(&(tp->done), &(tp->work_lock));
        } else{
            break;
        }
    }
    pthread_mutex_unlock(&(tp->work_lock));
}
