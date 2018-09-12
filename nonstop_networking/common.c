/**
* Networking Lab
* CS 241 - Spring 2018
*/

#include "common.h"

// Copy from lab chatroom

#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
// #include <user_hooks.h>
#include <stddef.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <netdb.h>

static const size_t MESSAGE_SIZE_DIGITS = 8;

char *create_message(char *name, char *message) {
    int name_len = strlen(name);
    int msg_len = strlen(message);
    char *msg = calloc(1, msg_len + name_len + 4);
    sprintf(msg, "%s: %s", name, message);

    return msg;
}

ssize_t get_message_size(int socket) {
    uint64_t size;
    ssize_t read_bytes =
        read_all_from_socket(socket, (char *)&size, MESSAGE_SIZE_DIGITS);
    if (read_bytes == 0 || read_bytes == -1)
        return read_bytes;

    return (ssize_t)size;
}

// You may assume size won't be larger than a 4 byte integer
ssize_t write_message_size(uint64_t size, int socket) {
    // Your code here
    // return 9001;
    // uint64_t temp = htonl((uint64_t)size);
    return write_all_to_socket(socket, (char*) &size, MESSAGE_SIZE_DIGITS);
}

ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    // Your Code Here
    size_t counter = 0;
    while (counter < count) {
      ssize_t temp = read(socket, buffer + counter, count - counter);
      if (!temp) {
        return counter;
      }
      if (temp == -1) {
        if (errno == EINTR) {
          continue;
        }
        else {
          return -1;
        }
      }
      counter += temp;
    }
    return counter;
}

ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    // Your Code Here
    size_t counter = 0;
    while (counter < count) {
      ssize_t temp = write(socket, buffer + counter, count - counter);
      if (!temp) {
        return counter;
      }
      if (temp == -1) {
        if (errno == EINTR) {
          continue;
        }
        else {
          return -1;
        }
      }
      counter += temp;
    }
    return counter;
}
