/**
 * charming_chatroom
 * CS 341 - Spring 2024
 */
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// I added:
#include <unistd.h>
#include <errno.h>

#include "utils.h"
static const size_t MESSAGE_SIZE_DIGITS = 4;

char *create_message(char *name, char *message) {
    int name_len = strlen(name);
    int msg_len = strlen(message);
    char *msg = calloc(1, msg_len + name_len + 4);
    sprintf(msg, "%s: %s", name, message);

    return msg;
}

ssize_t get_message_size(int socket) {
    int32_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);

    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;

    return (ssize_t)ntohl(size);
}

// You may assume size won't be larger than a 4 byte integer
ssize_t write_message_size(size_t size, int socket) {
    // Your code here

    int32_t msg_size = htonl(size);

    return write_all_to_socket(socket, (char *) &msg_size, MESSAGE_SIZE_DIGITS);

    //return 9001;
}

ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    // Your Code Here

    size_t num_read = 0;
    ssize_t result;

    while (num_read < count) {
        result = read(socket, (void*)buffer+num_read, count - num_read);
        //printf("result is %ld\n", result);
        if (result == 0) { return num_read; }
        else if (result > 0) { num_read += result; }
        else if (result == -1 && EINTR == errno) { continue; }
        else { perror(NULL); return -1; }
    }

    //printf("num_read is %ld\n", num_read);
    return num_read;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    // Your Code Here

    size_t total = 0;
    ssize_t result;

    while (total < count && (result = write(socket, (void*)buffer+total, count-total))) {
        // if interrupted, retry
        if (result == -1 && EINTR == errno) { continue; }

        // all other errors
        if (result == -1) { perror(NULL); return -1; }

        // if write is successful, add to 'total'
        total += result;
    }

    return total;
}
