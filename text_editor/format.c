/**
* Text Editor Lab
* CS 241 - Spring 2018
*/

#include "document.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_usage_error() {
    printf("\n  editor [-n] <filename>\n");
}

void invalid_command(char *command) {
    printf("'%s' is not a valid command!\n", command);
}

void print_line(document *document, size_t index, size_t start_col_index,
                ssize_t max_cols) {
    char *line = (char *)document_get_line(document, index);
    line = strdup(line);
    size_t line_len = strlen(line);
    for (size_t i = 0; i < line_len; i++) {
        if (line[i] == '\t' || line[i] == '\r') {
            line[i] = ' ';
        }
    }
    if (line_len == 0 || line_len < start_col_index) {
        printf("%zu\n", index);
    } else if (max_cols == -1) {
        printf("%zu\t%s\n", index, line + start_col_index);
    } else {
        printf("%zu\t%.*s\n", index, (int)max_cols, line + start_col_index);
    }
    free(line);
}

void print_document_empty_error() {
    fprintf(stderr, "This file has no lines to display!\n");
}

void found_search(char *search_str, size_t line_no, size_t idx) {
    fprintf(stderr, "Found '%s' at line %zu, col %zu\n", search_str, line_no,
            idx);
}

void not_found(char *search_str) {
    fprintf(stderr, "No results found for '%s'!\n", search_str);
}
