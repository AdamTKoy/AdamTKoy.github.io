/**
 * critical_concurrency
 * CS 341 - Spring 2024
 */
#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include <assert.h>

/**
 * This queue is implemented with a linked list of queue_nodes.
 */
typedef struct queue_node {
    void *data;
    struct queue_node *next;
} queue_node;

struct queue {
    /* queue_node pointers to the head and tail of the queue */
    queue_node *head, *tail;

    /* The number of elements in the queue */
    ssize_t size;

    ssize_t num_pull_trapped;
    ssize_t num_push_trapped;

    ssize_t stop_push;
    ssize_t stop_pull;

    /**
     * The maximum number of elements the queue can hold.
     * max_size is non-positive if the queue does not have a max size.
     */
    ssize_t max_size;

    /* Mutex and Condition Variable for thread-safety */
    pthread_cond_t cv;
    pthread_mutex_t m;

    pthread_cond_t cv2;
};

// static variables just for testing purposes:
// static ssize_t num_pulls = 0;
// static ssize_t num_pushes = 0;

queue *queue_create(ssize_t max_size) {
    /* Your code here */

    queue * q = malloc(sizeof(queue));

    q->head = NULL;
    q->tail = NULL;

    q->size = 0;
    q->max_size = max_size;

    q->num_pull_trapped = 0;
    q->num_push_trapped = 0;

    q->stop_push = 0;
    q->stop_pull = 0;

    pthread_mutex_init(&q->m, NULL);
    pthread_cond_init(&q->cv, NULL);
    pthread_cond_init(&q->cv2, NULL);
    
    return q;
}

void queue_destroy(queue *this) {
    /* Your code here */
    // assert(this);

    queue_node * current = this->head;
    queue_node * old = NULL;

    while (current) {
        old = current;
        current = current->next;
        free(old);
    }

    pthread_mutex_destroy(&this->m);
    pthread_cond_destroy(&this->cv);
    pthread_cond_destroy(&this->cv2);

    free(this);
}

void queue_push(queue *this, void *data) {
    /* Your code here */

    pthread_mutex_lock(&this->m);

    if (this->max_size > 0 && this->size == this->max_size) {
        // printf("queue full, TRAPPING PUSH thread\n");
        this->num_push_trapped++;
        
        while (this->max_size > 0 && this->size == this->max_size) {
            (pthread_cond_wait(&this->cv, &this->m));
        }
        // printf("trapped PUSH thread released\n");
        this->num_push_trapped--;
    }

    this->size++; 
    // num_pushes++;
    // printf("PUSHing: new queue size is %zd, num_pushes is %zd; ", this->size, num_pushes);

    queue_node * new_node = malloc(sizeof(queue_node));

    new_node->data = data;
    new_node->next = NULL;

    if (this->head == NULL) {
        this->head = new_node;
        this->tail = new_node;
    }

    else if (this->tail) {
        this->tail->next = new_node;
        this->tail = new_node;
    }

    if (this->num_pull_trapped > 0 && this->size > 0) {
        // printf("signaling pull, # trapped was %zd; ", this->num_pull_trapped);
        pthread_cond_signal(&(this->cv2));
    }

    // printf("now unlocking mutex after PUSH\n");
    pthread_mutex_unlock(&(this->m));

}

void *queue_pull(queue *this) {
    /* Your code here */

    pthread_mutex_lock(&(this->m));

    if (this->size == 0) {
        // printf("queue empty, TRAPPING PULL thread\n");
        this->num_pull_trapped++;        
        
        while (this->size == 0) {
            (pthread_cond_wait(&(this->cv2), &(this->m)));
        }
        // printf("trapped PULL thread released\n");
        this->num_pull_trapped--;
    }

    

    this->size--; 
    // num_pulls++;
    // printf("PULLing: new queue size is %zd, num_pulls is %zd; ", this->size, num_pulls);

    queue_node * new_head = NULL;
    queue_node * old_head = this->head;

    if (this->head->next) {     // not the only node in queue
        new_head = this->head->next;
    }

    else {      // removing only node
        this->tail = new_head;
    }
    
    void * val = this->head->data;

    this->head = new_head;
    
    if (this->max_size > 0 && this->num_push_trapped > 0 && this->size < this->max_size) {
        // printf("signaling push, # trapped was %zd; ", this->num_push_trapped);
        pthread_cond_signal(&(this->cv));
    }

    // printf("now unlocking mutex after PULL\n");
    pthread_mutex_unlock(&(this->m));

    free(old_head);
    old_head = NULL;

    return val;

}