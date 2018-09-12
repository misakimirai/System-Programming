/**
* Pointers Gone Wild Lab
* CS 241 - Spring 2018
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "callbacks.h"
#include "part3-functions.h"
#include "vector.h"

vector *vector_map(vector *input, mapper map) {
    // TODO implement this;
    vector *answer = string_vector_create();
    VECTOR_FOR_EACH(input, thing, {
      void *temp = map(thing);
      vector_push_back(answer, temp);
      free(temp);
    });
    return answer;
}

void *vector_reduce(vector *input, reducer reduce, void *acc) {
    // TODO implement this
    VECTOR_FOR_EACH(input, thing, {
      acc = reduce(thing, acc);
    });
    return acc;
}

void *length_reducer(char *input, void *output) {
    // TODO implement this
    if (!output) {
      output = (int *) malloc (sizeof(int));
      *((int*)output) = 0;
    }
    *((int*)output) += strlen(input);
    return (void*)output;
}

void *concat_reducer(char *input, void *output) {
    // TODO implement this
    if (!output) {
      output = malloc (strlen(input) + 1);
      strcpy(output, input);
    }
    else {
      output = realloc(output, strlen(input) + strlen(output) + 1);
      strcpy(output + strlen(output), input);
    }
    return (void*)output;
}
