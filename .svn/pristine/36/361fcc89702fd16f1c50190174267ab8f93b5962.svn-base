/**
* Pointers Gone Wild Lab
* CS 241 - Spring 2018
*/

#include <stdio.h>
#include <string.h>

#include "part3-functions.h"
#include "vector.h"

/**
 * Typedef allows us to turn the cumbursome function pointer
 * into something cleaner
 */
typedef void (*printer_type)(void *);

/**
 * See how much more readable vector_print's type signature looks comapred to:
 *  void vector_print(void(*printer)(void*), vector *v)
 */
void vector_print(printer_type printer, vector *v) {
    printf("[");
    VECTOR_FOR_EACH(v, elem, {
        printer(elem);
        if (_it + 1 != _iend)
            printf(", ");
    });
    printf("]\n");
}

// Hmm this looks like a printer type...
void printer(void *s) {
    printf("%s\n", (char *)s);
}

char *encoder_mapper(char *input) {
    char *output = strdup(input);
    for (size_t i = 0; i < strlen(output); i++)
        output[i] = output[i] ^ 13;
    return output;
}

int main(int argc, char **argv) {
    vector *argv_vector = string_vector_create();
    if (argc == 1) {
        printf("Usage: ./part3 [input strings]\n");
        printf("\tFor testing, try using './part3 testing part 3'\n");
        exit(1);
    }
    argv++;
    while (*argv) {
        vector_push_back(argv_vector, *argv);
        argv++;
    }
    vector_print(printer, argv_vector);

    vector *encoded = vector_map(argv_vector, encoder_mapper);
    if (!encoded)
        fprintf(stderr, "Result of first vector_map was %p!\n", encoded);
    else
        vector_print(printer, encoded);

    vector *decoded = vector_map(encoded, encoder_mapper);
    if (!decoded)
        fprintf(stderr, "Result of second vector_map was %p!\n", decoded);
    else
        vector_print(printer, decoded);

    if (encoded)
        vector_destroy(encoded);
    if (decoded)
        vector_destroy(decoded);

    int *length = (int *)vector_reduce(argv_vector, length_reducer, NULL);
    if (length)
        printf("length: %d\n", *length);
    else
        fprintf(stderr, "Result of vector_reduce with length_reducer was %p!\n",
                length);
    free(length);

    char *reduced = vector_reduce(argv_vector, concat_reducer, NULL);
    if (reduced)
        printf("%s\n", reduced);
    else
        fprintf(stderr, "Result of vector_reduce with concat_reducer was %p!\n",
                reduced);
    free(reduced);
    vector_destroy(argv_vector);

    return 0;
}
