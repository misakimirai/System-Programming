/**
* Teaching Threads Lab
* CS 241 - Spring 2018
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reduce.h"
#include "reducers.h"

/* You might need a struct for each task ... */
typedef struct task{
  pthread_t tid;
  int base_case;
  int *list;
  reducer reduce_func;
  int start;
  int end;
} task;

/* You should create a start routine for your threads. */
void* partial_task(void *para) {
  task *temp = (task*)para;
  int* result = malloc(sizeof(int));
  *result = temp->base_case;
  for (int i = temp->start; i < temp->end; i++) {
    // fprintf(stderr, "index %d value %d\n", i, temp->list[i]);
    *result = temp->reduce_func(*result, temp->list[i]);
  }
  // fprintf(stderr, "the end space is %d the result is %d\n", temp->end, *result);
  return (void*)result;
}

int par_reduce(int *list, size_t list_len, reducer reduce_func, int base_case,
               size_t num_threads) {
    /* Your implementation goes here */
    if (list_len < num_threads) {
      int result = base_case;
      for (size_t i = 0; i < list_len; ++i) {
          result = reduce_func(result, list[i]);
      }
      return result;
    }
    int num = list_len / num_threads;
    task* tasks[num_threads];
    for (size_t i = 0; i < num_threads; i++) {
      task *temp = (task*) malloc(sizeof(task));
      temp->base_case = base_case;
      temp->list = list;
      temp->reduce_func = reduce_func;
      temp->start = i * num;
      if (i != num_threads - 1) {
        temp->end = (i + 1) * num;
      }
      else {
        temp->end = list_len;
      }
      tasks[i] = temp;
      pthread_create(&(temp->tid), NULL, partial_task, (void*)temp);
    }
    // fprintf(stderr, "%d\n", __LINE__);
    int result = base_case;
    void* results[num_threads];
    for (size_t i = 0; i < num_threads; i++) {
      // fprintf(stderr, "%d\n", __LINE__);
      pthread_join(tasks[i]->tid, &results[i]);
      // fprintf(stderr, "%d\n", __LINE__);
      result = reduce_func(result, *((int*)results[i]));
      // fprintf(stderr, "%d\n", __LINE__);
      // sleep(1);
      free(results[i]);
    }
    for (size_t i = 0; i < num_threads; i++) {
      free(tasks[i]);
    }
    // fprintf(stderr, "%d\n", __LINE__);
    // for (size_t i = 0; i < list_len; ++i) {
    //     result = reduce_func(result, *((int*)results[i]));
    // }
    return result;
}
