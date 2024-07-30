/**
 * critical_concurrency
 * CS 341 - Spring 2024
 */
#include "barrier.h"

#include <stdio.h>

//static unsigned int done = 0;
//static pthread_mutex_t mtx2 = PTHREAD_MUTEX_INITIALIZER;
//static pthread_cond_t cv2 = PTHREAD_COND_INITIALIZER;

// The returns are just for errors if you want to check for them.
int barrier_destroy(barrier_t *barrier) {
    int error = 0;

    error = pthread_mutex_destroy(&(barrier->mtx));
    if (error != 0) return error;

    error = pthread_cond_destroy(&(barrier->cv));

    return error;

}

int barrier_init(barrier_t *barrier, unsigned int num_threads) {
    int error = 0;
    
    barrier->n_threads = num_threads;
    barrier->count = num_threads;
    barrier->times_used = 0;

    error = pthread_mutex_init(&(barrier->mtx), NULL);
    if (error != 0) return error;

    error = pthread_cond_init(&(barrier->cv), NULL);

    return error;
}

int barrier_wait(barrier_t *barrier) {

    pthread_mutex_lock(&(barrier->mtx));

    barrier->count--;

    if (barrier->count == 0) {
        barrier->times_used++;
        barrier->count = barrier->n_threads;
        pthread_cond_broadcast(&(barrier->cv));
    }
    else {
        while (pthread_cond_wait(&(barrier->cv), &(barrier->mtx)) != 0);
    }

    return pthread_mutex_unlock(&(barrier->mtx));

}
