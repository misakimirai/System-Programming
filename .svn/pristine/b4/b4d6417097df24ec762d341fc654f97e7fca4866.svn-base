/**
* Pointers Gone Wild Lab
* CS 241 - Spring 2018
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#pragma once
#include <string.h>

#include "callbacks.h"
#include "vector.h"

/**
 * typedefs to make the function signature of vector_map
 * and vector_reduce look nicer.
 *
 * To write your own typedef of a function pointer, the general
 * structure is as follows:
 *
 * (return type)(*type_name)(list of argument types)
 *
 * This will define a type with name 'type_name'.
 */
typedef char *(*mapper)(char *);
typedef void *(*reducer)(char *, void *);

/**
 * Maps the function map over a vector
 *
 * Pseudocode example:
 *
 * input = ["foo", "bar", "baz"]
 * vector_map(input, string_reverser) = ["oof", "rab", "zab"]
 *
 * Where we assume the existence of some function
 * char *string_reverser(char *input)  which takes a string as
 * input and returns a reversed version.
 */
vector *vector_map(vector *input, mapper map);

/**
 * Reduces a vector to one value with the function
 * specified by reduce and with an initial accumulator
 * value of acc.
 *
 * Pseudocode example:
 *
 * input = ["foo", "bar", "baz"]
 * *vector_reduce(input, length_reducer, NULL) = 9
 *
 * The example assumes that length_reducer has been implemented.
 */
void *vector_reduce(vector *input, reducer reduce, void *acc);

/**
 * Sum the lengths of all elements in a vector of strings.
 * The resulting integer should be allocated on the heap.
 *
 * Pseudocode example:
 *
 * input = ["foo", "bar", "baz"]
 * reduce(input, length_reducer, NULL) == 9
 *
 * For a more basic pseudocode example:
 *
 * int *length = malloc(sizeof(int));
 * *length = 3;
 * int *val = (int*)length_reducer("asdf", (void*)length)
 * *val == 7
 */
void *length_reducer(char *input, void *output);

/**
 * Join a vector of strings together and return the result.
 * The resulting string should be allocated on the heap.
 *
 * Pseudocode example:
 *
 * input = ["foo", "bar", "baz"]
 * reduce(input, concat_reducer, NULL) == "foobarbaz"
 *
 * For a more basic pseudocode example:
 *
 * char *inital = strdup("Hello");
 * (char*)concat_reducer(" world!", (void*)inital) == "Hello world!"
 */
void *concat_reducer(char *input, void *output);
