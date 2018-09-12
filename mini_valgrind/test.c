/**
* Mini Valgrind Lab
* CS 241 - Spring 2018
*/

#include <stdio.h>
#include <stdlib.h>

int main() {
    // Your tests here using malloc and free
    char *temp = malloc(5);
    int *temp1 = calloc(5, sizeof(int));
    free(temp + 1);
    temp = realloc(temp, 10);
    free(temp);
    return 0;
}
