/**
* Password Cracker Lab
* CS 241 - Spring 2018
*/

#include "cracker2.h"
#include "format.h"
#include "utils.h"

#include <crypt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "thread_status.h"

pthread_barrier_t end_barrier;
pthread_barrier_t start_barrier;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;;
pthread_cond_t cv;
int *count_array;
size_t total_thread;
int finish = 0;
char *curr_task;
char *password;
int writers;
int writing;
int reading;

void *helper(void *ptr) {
  size_t id = (size_t)ptr;
  struct crypt_data cdata;
  cdata.initialized = 0;
  while (1) {
    pthread_barrier_wait(&start_barrier);
    if (curr_task == NULL) {
        break;
    }
    char *temp_task = strdup(curr_task);
    char *task[3];
    char *saveptr;
    int x = 0;
    for (char *token = strtok_r(temp_task, " ", &saveptr); token != NULL; x++) {
        task[x] = strdup(token);
        token = strtok_r(NULL, " ", &saveptr);
    }
    char *name = task[0];
    char *hash = task[1];
    char *known = task[2];
    long start_index;
    long count;
    int length = getPrefixLength(known);
    getSubrange(strlen(known) - length, total_thread, (int)id, &start_index, &count);
    setStringPosition(known + length, start_index);
    v2_print_thread_start(id, name, start_index, known);

    long hash_count = 0;
    // use the reader writer to set the flag indicating weather finish
    for(; hash_count < count; hash_count++) {
      int temp = 0; // get the value of finish
      pthread_mutex_lock(&m);
      while (writers) {
        pthread_cond_wait(&cv, &m);
      }
      reading++;
      pthread_mutex_unlock(&m);
      temp = finish;
      pthread_mutex_lock(&m);
      reading--;
      pthread_cond_broadcast(&cv);
      pthread_mutex_unlock(&m);
      if (temp) {
        break;
      }
      char *hashed = crypt_r(known, "xx", &cdata);
      if (strcmp(hash, hashed) == 0) {
        // find the password
        password = strdup(known);
        pthread_mutex_lock(&m);
        writers++;
        while (reading || writing) {
          pthread_cond_wait(&cv, &m);
        }
        writing++;
        pthread_mutex_unlock(&m);
        finish = (int)id;
        pthread_mutex_lock(&m);
        writing--;
        writers--;
        pthread_cond_broadcast(&cv);
        pthread_mutex_unlock(&m);
        // remember the last run of hash_count
        hash_count++;
        break;
      }
      else {
        incrementString(known + length);
      }
    }
    // check the condition
    int print_result = 2;
    if (finish == (int)id) {
      print_result = 0;
    }
    else if (hash_count < count - 1) {
      print_result = 1;
    }
    count_array[id - 1] = hash_count;
    v2_print_thread_result(id, hash_count, print_result);
    free(temp_task);
    free(name);
    free(hash);
    free(known);
    pthread_barrier_wait(&end_barrier);
  }
  return NULL;
}

int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads
    // remember the main thread
    pthread_barrier_init(&end_barrier, NULL, thread_count + 1);
    pthread_barrier_init(&start_barrier, NULL, thread_count + 1);
    pthread_cond_init(&cv, NULL);
    count_array = (int *)malloc(thread_count * sizeof(int));
    total_thread = thread_count;
    pthread_t threads[thread_count];
    for (size_t i = 0; i < thread_count; i++) {
      pthread_create(threads + i, NULL, helper, (void*)(i + 1));
    }
    char *buffer = NULL;
    size_t capacity = 0;
    while (1) {
      double time_start = getTime();
      double time_start_cpu = getCPUTime();
      ssize_t size = getline(&buffer, &capacity, stdin);
      if (size == -1) {
        curr_task = NULL;
        free(buffer);
        pthread_barrier_wait(&start_barrier);
        break;
      }
      else {
        if (size > 0 && buffer[size - 1] == '\n') {
          buffer[size - 1] = '\0';
        }
        curr_task = strdup(buffer);
        char *name = strtok(buffer, " ");
        v2_print_start_user(name);
        finish = 0;
        pthread_barrier_wait(&start_barrier);
        pthread_barrier_wait(&end_barrier);
        int total = 0;
        for (size_t i = 0; i < thread_count; i++) {
            total += count_array[i];
        }
        int print_finish = 1;
        if (finish > 0) {
          print_finish = 0;
        }
        v2_print_summary(name, password, total, getTime() - time_start, getCPUTime() - time_start_cpu, print_finish);
        if (finish) {
          free(password);
        }
        free(curr_task);
      }
    }
    void *temp[thread_count];
    for (size_t i = 0; i < thread_count; i++) {
        pthread_join(threads[i], &temp[i]);
    }
    free(count_array);
    pthread_barrier_destroy(&end_barrier);
    pthread_barrier_destroy(&start_barrier);
    pthread_mutex_destroy(&m);
    pthread_cond_destroy(&cv);
    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
