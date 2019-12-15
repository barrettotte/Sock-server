#ifndef __THPOOL_H__
#define __THPOOL_H__

#include <stddef.h>
#include <stdbool.h>


struct thpool;
typedef struct thpool thpool_t;
typedef void (*thread_func_t)(void *arg);


/* Create thread pool */
thpool_t *thpool_create(size_t i);


/* Enqueue work to be done by thread in pool */
bool thpool_add_work(thpool_t *tm, thread_func_t func, void *arg);


/* Wait for all queued work to complete */
void thpool_wait(thpool_t *tm);


/* Cancel all queued work and destroy pool */
void thpool_destroy(thpool_t *tm);

#endif