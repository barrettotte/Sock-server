#include <stdlib.h>
#include <pthread.h>
#include "thpool.h"


struct thpool_work{
    thread_func_t       func;
    void               *arg;
    struct thpool_work *next;
};
typedef struct thpool_work thpool_work_t;

struct thpool{
    thpool_work_t   *work_first;
    thpool_work_t   *work_last;
    pthread_mutex_t  work_mutex;   // Lock used by all threads, synchronize work fetching
    pthread_cond_t   work_cond;    // signal when there is work to be processed
    pthread_cond_t   working_cond; // signal when there are no threads processing
    size_t           working_cnt;  // active threads
    size_t           thread_cnt;   // alive threads
    bool             stop;
};


static thpool_work_t *thpool_work_create(thread_func_t func, void *arg){
    thpool_work_t *work;

    if(func == NULL){
        return NULL;
    }
    work = malloc(sizeof(*work));
    work->func = func;
    work->arg = arg;
    work->next = NULL;
    return work;
}

static void thpool_work_destroy(thpool_work_t *work){
    if(work == NULL){
        return;
    }
    free(work);
}

static thpool_work_t *thpool_work_get(thpool_t *tm){
    thpool_work_t *work;

    if(tm == NULL){
        return NULL;
    }
    work = tm->work_first;
    if(work == NULL){
        return NULL;
    }

    if(work->next == NULL){
        tm->work_first = NULL;
        tm->work_last = NULL;
    } else {
        tm->work_first = work->next;
    }
    return work;
}

static void *thpool_worker(void *arg){
    thpool_t *tm = arg;
    thpool_work_t *work;
    while(1){
        pthread_mutex_lock(&(tm->work_mutex));
        if(tm->stop){
            break;
        }

        // Check for available work, wait around if none
        if(tm->work_first == NULL){
            pthread_cond_wait(&(tm->work_cond), &(tm->work_mutex));
            // conditional unlocks and auto relocks mutex when signaled
        }

        // Perform work
        work = thpool_work_get(tm);
        tm->working_cnt++;
        pthread_mutex_unlock(&(tm->work_mutex));
        if(work != NULL){
            work->func(work->arg);
            thpool_work_destroy(work);
        }
        
        // Finish work
        pthread_mutex_lock(&(tm->work_mutex));
        tm->working_cnt--;
        if(!tm->stop && tm->working_cnt == 0 && tm->work_first == NULL){
            pthread_cond_signal(&(tm->working_cond)); // wake up threads
        }
        pthread_mutex_unlock(&(tm->work_mutex));
    }
    // Stop thread
    tm->thread_cnt--;
    pthread_cond_signal(&(tm->working_cond));
    pthread_mutex_unlock(&(tm->work_mutex));
    return NULL;
}

thpool_t *thpool_create(size_t size){
    thpool_t *tm;
    pthread_t thread;
    if(size == 0){
        size = 2;
    }

    tm = calloc(1, sizeof(*tm));
    tm->thread_cnt = size;
    pthread_mutex_init(&(tm->work_mutex), NULL);
    pthread_cond_init(&(tm->work_cond), NULL);
    pthread_cond_init(&(tm->working_cond), NULL);
    tm->work_first = NULL;
    tm->work_last = NULL;

    // Create threads for pool
    for(int i = 0; i < size; i++){
        pthread_create(&thread, NULL, thpool_worker, tm);
        pthread_detach(thread);
    }
    return tm;
}

void thpool_destroy(thpool_t *tm){
    thpool_work_t *work;
    thpool_work_t *worktmp;
    if(tm == NULL){
        return;
    }
    
    // Finish active processes
    pthread_mutex_lock(&(tm->work_mutex));
    work = tm->work_first;
    while(work != NULL){
        worktmp = work->next;
        thpool_work_destroy(work);
        work = worktmp;
    }

    tm->stop = true;
    pthread_cond_broadcast(&(tm->work_cond));
    pthread_mutex_unlock(&(tm->work_mutex));
    
    thpool_wait(tm); // Wait in case threads are still processing
    pthread_mutex_destroy(&(tm->work_mutex));
    pthread_cond_destroy(&(tm->work_cond));
    pthread_cond_destroy(&(tm->working_cond));
    free(tm);
}

bool thpool_add_work(thpool_t *tm, thread_func_t func, void *arg){
    thpool_work_t *work;
    if(tm == NULL){
        return false;
    }
    work = thpool_work_create(func, arg);
    if(work == NULL){
        return false;
    }

    // enqueue new work
    pthread_mutex_lock(&(tm->work_mutex));
    if(tm->work_first == NULL){
        tm->work_first = work;
        tm->work_last = tm->work_first;
    } else{
        tm->work_last->next = work;
        tm->work_last = work;
    }
    pthread_cond_broadcast(&(tm->work_cond));
    pthread_mutex_unlock(&(tm->work_mutex));
    return true;
}

void thpool_wait(thpool_t *tm){
    if(tm == NULL){
        return;
    }
    pthread_mutex_lock(&(tm->work_mutex));
    while(1){
        if((!tm->stop && tm->working_cnt != 0) || (tm->stop && tm->thread_cnt != 0)){
            pthread_cond_wait(&(tm->working_cond), &(tm->work_mutex));
        } else{
            break;
        }
    }
    pthread_mutex_unlock(&(tm->work_mutex));
}
