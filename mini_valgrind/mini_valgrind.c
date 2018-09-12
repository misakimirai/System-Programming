/**
* Mini Valgrind Lab
* CS 241 - Spring 2018
*/

#include "mini_valgrind.h"
#include <stdio.h>
#include <string.h>


meta_data *head;
size_t total_memory_requested;
size_t total_memory_freed;
size_t invalid_addresses;


void *mini_malloc(size_t request_size, const char *filename,
                  void *instruction) {
    // your code here
    if (!request_size)
      return NULL;
    meta_data *temp = malloc(request_size + sizeof(meta_data));
    if (!temp) {
      return NULL;
    }
    total_memory_requested += request_size;
    temp->request_size = request_size;
    temp->filename = filename;
    temp->instruction = instruction;
    temp->next = head;
    head = temp;
    return temp + 1;
}

void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {
    // your code here
    if (!num_elements || !element_size)
      return NULL;
    size_t request_size = num_elements * element_size;
    meta_data *temp = calloc(request_size + sizeof(meta_data), 1);
    if (!temp) {
      return NULL;
    }
    total_memory_requested += request_size;
    temp->request_size = request_size;
    temp->filename = filename;
    temp->instruction = instruction;
    temp->next = head;
    head = temp;
    return temp + 1;
}

void *mini_realloc(void *payload, size_t request_size, const char *filename,
                   void *instruction) {
    // your code here
    if (!payload) {
      return mini_malloc(request_size, filename, instruction);
    }
    if (!request_size) {
      mini_free(payload);
      return NULL;
    }
    meta_data *target = (meta_data*)payload - 1;
    meta_data *temp = head;
    meta_data *temp1 = NULL;
    while (temp) {
      if (temp == target) {
        // fprintf(stderr, "%d %zu\n", __LINE__, temp->request_size);
        meta_data *result = realloc(temp, request_size + sizeof(meta_data));
        if (!result) {
          return NULL;
        }
        if (result->request_size < request_size) {
          // fprintf(stderr, "%d %lu\n", __LINE__, request_size - result->request_size);
          total_memory_requested += (request_size - result->request_size);
        }
        else {
          // fprintf(stderr, "%d %lu\n", __LINE__, result->request_size - request_size);
          total_memory_freed += (result->request_size - request_size);
        }
        if (temp1 == NULL) {
          head = result;
        }
        else {
          temp1->next = result;
        }
        result->request_size = request_size;
        result->filename = filename;
        result->instruction = instruction;
        result->next = temp->next;
        return result + 1;
      }
      temp1 = temp;
      temp = temp->next;
    }
    invalid_addresses++;
    return NULL;
}

void mini_free(void *payload) {
    // your code here
    if (!payload)
      return;
    meta_data *target = (meta_data*)payload - 1;
    meta_data *temp = head;
    meta_data *temp1 = NULL;
    while(temp) {
      if (temp == target) {
        if (temp == head) {
          head = head->next;
        }
        else {
          temp1->next = temp->next;
        }
        total_memory_freed += temp->request_size;
        free(temp);
        return;
      }
      temp1 = temp;
      temp = temp->next;
    }
    invalid_addresses++;
}
