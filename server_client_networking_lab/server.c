/**
 * charming_chatroom
 * CS 341 - Spring 2024
 */
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils.h"

#define MAX_CLIENTS 8

void *process_client(void *p);

static volatile int serverSocket;
static volatile int endSession;

static volatile int clientsCount;
static volatile int clients[MAX_CLIENTS];

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * Signal handler for SIGINT.
 * Used to set flag to end server.
 */
void close_server() {
    pthread_mutex_lock(&mutex);
    endSession = 1;
    pthread_mutex_unlock(&mutex);
    // add any additional flags here you want.
}

/**
 * Cleanup function called in main after `run_server` exits.
 * Server ending clean up (such as shutting down clients) should be handled
 * here.
 */
void cleanup() {
    if (shutdown(serverSocket, SHUT_RDWR) != 0) {
        perror("shutdown():");
    }
    close(serverSocket);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1) {
            if (shutdown(clients[i], SHUT_RDWR) != 0) {
                perror("shutdown(): ");
            }
            close(clients[i]);
        }
    }
}

/**
 * Sets up a server connection.
 * Does not accept more than MAX_CLIENTS connections.  If more than MAX_CLIENTS
 * clients attempts to connects, simply shuts down
 * the new client and continues accepting.
 * Per client, a thread should be created and 'process_client' should handle
 * that client.
 * Makes use of 'endSession', 'clientsCount', 'client', and 'mutex'.
 *
 * port - port server will run on.
 *
 * If any networking call fails, the appropriate error is printed and the
 * function calls exit(1):
 *    - fprtinf to stderr for getaddrinfo
 *    - perror() for any other call
 */
void run_server(char *port) {
    /*QUESTION 1*/
    /*QUESTION 2*/
    /*QUESTION 3*/
    /*QUESTION 4*/
    /*QUESTION 5*/
    /*QUESTION 6*/
    struct addrinfo hints, * result;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    int s = getaddrinfo(NULL, port, &hints, &result);
    if (s != 0) { fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s)); exit(1); }

    int serverSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (serverSocket < 0) { perror(NULL); exit(1); }

    /*QUESTION 8*/
    int optval = 1;
    int retval = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    if (retval == -1) { perror(NULL); exit(1); }

    int optval2 = 1;
    int retval2 = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval2, sizeof(optval2));
    if (retval2 == -1) { perror(NULL); exit(1); }

    /*QUESTION 9*/
    if (bind(serverSocket, result->ai_addr, result->ai_addrlen) != 0) { perror(NULL); exit(1); }

    /*QUESTION 10*/
    if (listen(serverSocket, MAX_CLIENTS) != 0) { perror(NULL); exit(1); }

    clientsCount = 0;
    endSession = 0;

    // initialize all values of 'clients' array to -1
    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = -1;
    }

    /*QUESTION 11*/
    while (1) {
        // if this flag is set, server can finish
        // 'graceful exit' handled by 'cleanup' function
        pthread_mutex_lock(&mutex);
        if (endSession) { pthread_mutex_unlock(&mutex); break; }
        pthread_mutex_unlock(&mutex);

        // do I need something for the last 2 args?
        int client_fd = accept(serverSocket, NULL, NULL);
        if (client_fd < 0) { perror(NULL); exit(1); }

        //printf("client_fd is %d\n", client_fd);

        // create thread for each client
        // pthread_create(&threads[0], NULL, write_to_server, (void *)argv[3]);
        // process_client(void *p) where p is the index of client fd in 'clients' array

        // If more than MAX_CLIENTS clients attempts to connect, 
        // shut down new client and continue accepting.
        pthread_mutex_lock(&mutex);
        if (clientsCount == MAX_CLIENTS) {
            shutdown(client_fd, SHUT_RDWR);
            close(client_fd);
            pthread_mutex_unlock(&mutex);
        }
        else {
            //printf("server: adding client\n");
            intptr_t a = 0;
            //int i;
            // find available index
            for (int i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i] == -1) {
                    clients[i] = client_fd;
                    a = (intptr_t) i;
                    clientsCount++;
                    break;
                }
            }

            pthread_mutex_unlock(&mutex);

            pthread_t new_thread;
            pthread_create(&new_thread, NULL, process_client, (void*)a);
            
            //printf("server: client thread created, clientsCount is %d\n", clientsCount);

            pthread_join(new_thread, NULL);
        }
    }
}

/**
 * Broadcasts the message to all connected clients.
 *
 * message  - the message to send to all clients.
 * size     - length in bytes of message to send.
 */
void write_to_clients(const char *message, size_t size) {
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1) {
            ssize_t retval = write_message_size(size, clients[i]);
            if (retval > 0) {
                retval = write_all_to_socket(clients[i], message, size);
            }
            if (retval == -1) {
                perror("write(): ");
            }
        }
    }
    pthread_mutex_unlock(&mutex);
}

/**
 * Handles the reading to and writing from clients.
 *
 * p  - (void*)intptr_t index where clients[(intptr_t)p] is the file descriptor
 * for this client.
 *
 * Return value not used.
 */
void *process_client(void *p) {
    pthread_detach(pthread_self());
    intptr_t clientId = (intptr_t)p;
    ssize_t retval = 1;
    char *buffer = NULL;

    while (retval > 0 && endSession == 0) {
        //printf("thread about to get_message_size, clients[%ld] is %d\n", clientId, clients[clientId]);
        retval = get_message_size(clients[clientId]);
        //printf("message size was %zu\n", retval);
        if (retval > 0) {
            buffer = calloc(1, retval);
            //printf("thread about to read_all_from_socket...\n");
            retval = read_all_from_socket(clients[clientId], buffer, retval);
        }
        if (retval > 0) {
            //printf("thread about to write_to_clients...\n");
            write_to_clients(buffer, retval);
        }

        //printf("thread about to free buffer...\n");

        free(buffer);
        buffer = NULL;
    }

    printf("User %d left\n", (int)clientId);
    close(clients[clientId]);

    pthread_mutex_lock(&mutex);
    clients[clientId] = -1;
    clientsCount--;
    pthread_mutex_unlock(&mutex);

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "%s <port>\n", argv[0]);
        return -1;
    }

    struct sigaction act;
    memset(&act, '\0', sizeof(act));
    act.sa_handler = close_server;
    if (sigaction(SIGINT, &act, NULL) < 0) {
        perror("sigaction");
        return 1;
    }

    run_server(argv[1]);
    cleanup();
    pthread_exit(NULL);
}
