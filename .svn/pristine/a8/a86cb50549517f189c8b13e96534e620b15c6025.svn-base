/**
* Text Editor Lab
* CS 241 - Spring 2018
*/

#include "document.h"
#include "editor.h"
#include "format.h"
#include "sstream.h"

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CSTRING(x) ((bytestring){(char *)x, -1})
#define DECLARE_BUFFER(name) bytestring name = {NULL, 0};
#define EMPTY_BYTES ((bytestring){NULL, 0})

char *get_filename(int argc, char *argv[]) {
    // TODO implement get_filename
    // take a look at editor_main.c to see what this is used for
    return NULL;
}

sstream *handle_create_stream() {
    // TODO create empty stream
    return NULL;
}
document *handle_create_document(const char *path_to_file) {
    // TODO create the document
    return NULL;
}

void handle_cleanup(editor *editor) {
    // TODO destroy the document
}

void handle_display_command(editor *editor, size_t start_line,
                            ssize_t max_lines, size_t start_col_index,
                            ssize_t max_cols) {
    // TODO implement handle_display_command
}

void handle_insert_command(editor *editor, location loc, const char *line) {
    // TODO implement handle_insert_command
}

void handle_append_command(editor *editor, size_t line_no, const char *line) {
    // TODO implement handle_append_command
}

void handle_write_command(editor *editor, size_t line_no, const char *line) {
    // TODO implement handle_write_command
}

void handle_delete_command(editor *editor, location loc, size_t num_chars) {
    // TODO implement handle_delete_command
}

void handle_delete_line(editor *editor, size_t line_no) {
    // TODO implement handle_delete_line
}

location handle_search_command(editor *editor, location loc,
                               const char *search_str) {
    // TODO implement handle_search_command
    return (location){0, 0};
}

void handle_merge_line(editor *editor, size_t line_no) {
    // TODO implement handle_merge_line
}

void handle_split_line(editor *editor, location loc) {
    // TODO implement handle_split_line
}

void handle_save_command(editor *editor) {
    // TODO implement handle_save_command
}
