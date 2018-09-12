/**
* Text Editor Lab
* CS 241 - Spring 2018
*/

#include "display.h"
#include "document.h"
#include "editor.h"
#include "exts/extensions.h"
#include "format.h"
#include "sstream.h"

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <ncurses.h>
#include <regex.h>

#define EXIT     \
    (location) { \
        0, 1     \
    }
#define CONTINUE \
    (location) { \
        0, 0     \
    }

#define UNPACK_EDITOR(x)              \
    document *document = x->document; \
    sstream *stream = x->stream;      \
    char *filename = x->filename;     \
    (void)document;                   \
    (void)stream;                     \
    (void)filename;
#define CSTRING(x) ((bytestring){(char *)x, -1})
#define DECLARE_BUFFER(name) bytestring name = {NULL, 0};
#define EMPTY_BYTES ((bytestring){NULL, 0})

// Set up variables to hold regex
static regex_t write_or_append;
static regex_t insert;
static regex_t print_or_delete;
static regex_t complex_print;
static regex_t delete;
static regex_t merge;
static regex_t split;
static regex_t search_in_text;
static regex_t one_char_command;
static int setup_done = 0;

static void regex_error(int error) {
    if (error) {
        fprintf(stderr, "Failed to compile regex!\n");
        exit(1);
    }
}

int setup() {
    if (setup_done)
        return 0;
    // Compile regex
    // Pattern for writes and appends
    int regerr =
        regcomp(&write_or_append, "(w|a) [1-9][0-9]* .*$", REG_EXTENDED);
    regex_error(regerr);
    // Insert
    regerr = regcomp(&insert, "i [1-9][0-9]* [0-9]+ .*$", REG_EXTENDED);
    regex_error(regerr);
    // Patterns in the form p 30 or d 10
    regerr = regcomp(&print_or_delete, "(p|d) [1-9][0-9]*$", REG_EXTENDED);
    regex_error(regerr);
    // Control display behavior
    regerr = regcomp(&complex_print, "p [1-9][0-9]* [0-9]+ -?[0-9]+ -?[0-9]+$",
                     REG_EXTENDED);
    regex_error(regerr);
    // Delete from index
    regerr = regcomp(&delete, "d [1-9][0-9]* [0-9]+ [0-9]+$", REG_EXTENDED);
    regex_error(regerr);
    // Merge lines
    regerr = regcomp(&merge, "m [1-9][0-9]*$", REG_EXTENDED);
    regex_error(regerr);
    // Split lines
    regerr = regcomp(&split, "sp [1-9][0-9]* [0-9]+$", REG_EXTENDED);
    regex_error(regerr);
    // Search pattern, a backslash prepends the argument
    regerr = regcomp(&search_in_text, "f [1-9][0-9]* [0-9]+ .+$", REG_EXTENDED);
    regex_error(regerr);
    // One character patterns for save, print and quit
    regerr = regcomp(&one_char_command, "(s|p|q)$", REG_EXTENDED);
    regex_error(regerr);

    setup_done = 1;
    return 0;
}

void parser_cleanup() {
    if (setup_done) {
        regfree(&write_or_append);
        regfree(&insert);
        regfree(&print_or_delete);
        regfree(&complex_print);
        regfree(&delete);
        regfree(&merge);
        regfree(&split);
        regfree(&search_in_text);
        regfree(&one_char_command);
        setup_done = 0;
    }
}

char *get_location(char *input, location *loc) {
    loc->line_no = (size_t)strtol(input, &input, 10);
    input++;
    loc->idx = (size_t)strtol(input, &input, 10);
    input++;
    return input;
}

location handle_command(editor *editor, char *command) {
    UNPACK_EDITOR(editor);
    setup();
    // Remove trailing newline
    if (command[strlen(command) - 1] == '\n') {
        command[strlen(command) - 1] = '\0';
    }
    if (!regexec(&write_or_append, command, 0, NULL, 0)) {
        char wOrA = command[0];
        char *new_string = NULL;
        int lineno = (int)strtol(command + 2, &new_string, 10);
        new_string++;
        if (wOrA == 'w') {
            handle_write_command(editor, lineno, new_string);
        } else {
            handle_append_command(editor, lineno, new_string);
        }
    } else if (!regexec(&insert, command, 0, NULL, 0)) {
        location loc;
        char *new_string = get_location(command + 2, &loc);

        size_t line_count = document_size(document);
        if (loc.line_no > line_count && loc.idx)
            invalid_command(command);
        else if (loc.line_no > line_count)
            handle_insert_command(editor, loc, new_string);
        else {
            char const *curr_line = document_get_line(document, loc.line_no);
            if (loc.idx > strlen(curr_line))
                invalid_command(command);
            else
                handle_insert_command(editor, loc, new_string);
        }
    } else if (!regexec(&print_or_delete, command, 0, NULL, 0)) {
        char *end = NULL;
        int lineno = (int)strtol(command + 2, &end, 10);
        if (command[0] == 'p')
            handle_display_command(editor, MAX(3, lineno) - 2, 5, 0, -1);
        else
            handle_delete_line(editor, lineno);
    } else if (!regexec(&complex_print, command, 0, NULL, 0)) {
        location loc;
        char *end = get_location(command + 2, &loc);
        ssize_t max_lines, max_cols;
        sscanf(end, "%zd %zd\n", &max_lines, &max_cols);
        handle_display_command(editor, loc.line_no, max_lines, loc.idx,
                               max_cols);
    } else if (!regexec(&delete, command, 0, NULL, 0)) {
        location loc;
        char *end = get_location(command + 2, &loc);
        size_t num_chars = (size_t)strtol(end, &end, 10);

        if (loc.line_no > document_size(document) ||
            loc.idx + num_chars >
                strlen(document_get_line(document, loc.line_no)))
            invalid_command(command);
        else
            handle_delete_command(editor, loc, num_chars);
    } else if (!regexec(&merge, command, 0, NULL, 0)) {
        char *end = command + 2;
        size_t lineno = (size_t)strtol(end, &end, 10);
        if (lineno >= document_size(document))
            invalid_command(command);
        else
            handle_merge_line(editor, lineno);
    } else if (!regexec(&split, command, 0, NULL, 0)) {
        location loc;
        get_location(command + 3, &loc);

        if (loc.line_no > document_size(document) ||
            loc.idx > strlen(document_get_line(document, loc.line_no)))
            invalid_command(command);
        else
            handle_split_line(editor, loc);
    } else if (!regexec(&search_in_text, command, 0, NULL, 0)) {
        location loc;
        char *end = get_location(command + 2, &loc);
        location res = handle_search_command(editor, loc, end);
        if (res.line_no != 0) {
            found_search(end, res.line_no, res.idx);
            return res;
        } else {
            not_found(end);
        }
    } else if (!regexec(&one_char_command, command, 0, NULL, 0)) {
        if (command[0] == 's') {
            handle_save_command(editor);
        } else if (command[0] == 'q') {
            handle_cleanup(editor);
            parser_cleanup();
            return EXIT;
        } else {
            handle_display_command(editor, 1, -1, 0, -1);
        }
    } else {
        invalid_command(command);
    }
    return CONTINUE;
}

static short load_extensions = 0;

int get_editor(char *filename, editor *editor) {
    document *document = handle_create_document(filename);
    if (!document) {
        fprintf(stderr, "Document was NULL!\n");
        return 1;
    }
    sstream *stream = handle_create_stream();
    editor->document = document;
    editor->stream = stream;
    editor->filename = filename;
    return 0;
}

/**
 * This is the will be the entry point to your text editor.
*/
int entry_point(char *name) {
    editor editor;
    int editor_ret = get_editor(name, &editor);
    if (editor_ret)
        exit(1);
    UNPACK_EDITOR((&editor));
    while (1) {
        char *input_buffer = NULL;
        size_t n = 0;
        ssize_t s = getline(&input_buffer, &n, stdin);
        if (s < 0) {
            handle_cleanup(&editor);
            exit(0);
        }
        location quit = handle_command(&editor, input_buffer);
        free(input_buffer);
        if (quit.line_no == EXIT.line_no && quit.idx == EXIT.idx)
            exit(0);
    }
}

int ncurses_entry_point(char *name) {
    editor editorobj;
    editor *editor = &editorobj;
    int editor_ret = get_editor(name, editor);
    if (editor_ret)
        exit(1);
    UNPACK_EDITOR(editor);
    // Extensions :D
    char *buffer[100];

    display *display = display_create(editor, handle_command);

    char *header = NULL;
    asprintf(&header, "[%s]\n", filename);
    display_set_header(display, header);

    int done = 0;
    char *footer = NULL;
    char input_str[2];
    input_str[1] = 0;
    while (!done) {
        display_refresh_text(display);
        if (footer) {
            free(footer);
            footer = NULL;
        }

        int c = display_get_char(display);
        if (c < 0) {
            continue;
        }

        location real_loc = display_get_loc(display);
        switch (c) {
        case KEY_UP: {
            display_cursor_up(display);
        } break;
        case KEY_DOWN: {
            display_cursor_down(display);
        } break;
        case KEY_LEFT: {
            display_cursor_left(display);
        } break;
        case KEY_RIGHT: {
            display_cursor_right(display);
        } break;
        case KEY_BACKSPACE: {
            if (real_loc.idx >= 1) {
                char *command = NULL;
                asprintf(&command, "d %zu %zu %d", real_loc.line_no,
                         real_loc.idx - 1, 1);
                handle_command(editor, command);
                free(command);
                display_cursor_left(display);
            } else if (real_loc.line_no != 1) {
                // cursor is on first column, so merge this line with the
                // previous one
                size_t newidx =
                    strlen(document_get_line(document, real_loc.line_no - 1));
                char *command = NULL;
                asprintf(&command, "m %zu", real_loc.line_no - 1);
                handle_command(editor, command);
                free(command);
                display_cursor_up(display);
                // Update cursor index
                if (newidx - display->start_col_idx >
                    display->col - TAB_WIDTH - 2) {
                    display->start_col_idx =
                        newidx - (display->col - TAB_WIDTH - 2);
                    (display->loc).idx = display->col - TAB_WIDTH - 2;
                } else {
                    (display->loc).idx = newidx - display->start_col_idx;
                }

                asprintf(&footer, "merged: %zu", real_loc.line_no);
                display_set_footer(display, footer);
            }
        } break;
        case KEY_DC: {
            if (real_loc.idx !=
                strlen(document_get_line(document, real_loc.line_no))) {
                char *command = NULL;
                asprintf(&command, "d %zu %zu %d", real_loc.line_no,
                         real_loc.idx, 1);
                handle_command(editor, command);
                free(command);
            } else if (real_loc.line_no != document_size(document)) {
                // cursor is on last column, so merge this line with the next
                // one
                char *command = NULL;
                asprintf(&command, "m %zu", real_loc.line_no);
                handle_command(editor, command);
                free(command);
                asprintf(&footer, "merged: %zu", real_loc.line_no + 1);
                display_set_footer(display, footer);
            }
        } break;
        case KEY_HOME: {
            (display->loc).idx = 0;
            display->start_col_idx = 0;
        } break;
        case KEY_END: {
            size_t line_len =
                strlen(document_get_line(document, real_loc.line_no));
            if (line_len - display->start_col_idx > display->col - TAB_WIDTH) {
                // end of line is on the right side of display
                display->start_col_idx =
                    line_len - (display->col - TAB_WIDTH - 2);
                (display->loc).idx = display->col - TAB_WIDTH - 2;
            } else {
                (display->loc).idx = line_len - display->start_col_idx;
            }
        } break;
        case 10: { // KEY_ENTER
            if (document_size(document)) {
                char *command = NULL;
                asprintf(&command, "sp %zu %zu", real_loc.line_no,
                         real_loc.idx);
                handle_command(editor, command);
                free(command);
                (display->loc).idx = 0;
                display->start_col_idx = 0;
            } else {
                char *command = NULL;
                asprintf(&command, "i %zu %d ", real_loc.line_no + 1, 0);
                handle_command(editor, command);
                free(command);
            }
            display_cursor_down(display);
        } break;
        case 27: {
            c = display_get_char(display);
            switch (c) {
                /**
                 *  OPTIONAL: Extending the TUI's functionality
                 *
                 *  You can add your own features here! Just read the file
                 * exts/Instructions.md
                 *  to find out how.
                */
                EXTENSIONS(c);
            }
        } break;
        default: {
            if ((c >= 32) && (c <= 126)) {
                char *command = NULL;
                asprintf(&command, "i %zu %zu %c", real_loc.line_no,
                         real_loc.idx, c);
                handle_command(editor, command);
                free(command);
                display_cursor_right(display);
                break;
            }
            const char *key_name = keyname(c);
            if (key_name[0] == '^') {
                // CTRL was pressed!
                switch (key_name[1]) {
                case 'A': {
                    display_destroy(display);
                    handle_cleanup(editor);
                    done = 1;
                } break;
                case 'X': {
                    handle_command(editor, "s");
                    asprintf(&footer, "Wrote file to: %s", filename);
                    display_set_footer(display, footer);
                } break;
                case 'W': {
                    // delete line with ctrl + w
                    char *command = NULL;
                    asprintf(&command, "d %zu", real_loc.line_no);
                    handle_command(editor, command);
                    free(command);
                    if (real_loc.line_no > document_size(document)) {
                        if (document_size(document) != 0) {
                            display_cursor_up(display);
                        } else {
                            (display->loc).idx = 0;
                            display->start_col_idx = 0;
                        }
                    } else {
                        display_cursor_idx_check(display);
                    }
                    asprintf(&footer, "Deleted line: %zu", real_loc.line_no);
                    display_set_footer(display, footer);
                } break;
                case 'F': {
                    // ctrl+f to search
                    char *search_str = NULL;
                    display_interact(display, "Enter Search String:",
                                     &search_str);
                    if (search_str) {
                        // offsetting idx by 1 to not return the same location
                        // incase the user is already
                        // on top of a search result
                        char *command = NULL;
                        asprintf(&command, "f %zu %zu %s", real_loc.line_no,
                                 real_loc.idx + 1, search_str);
                        location _search_line_buffer =
                            handle_command(editor, command);
                        free(command);
                        if (_search_line_buffer.line_no) {
                            display_set_loc(display,
                                            _search_line_buffer.line_no,
                                            _search_line_buffer.idx);
                            asprintf(&footer, "Found %s at line: %zu char: %zu",
                                     search_str, _search_line_buffer.line_no,
                                     _search_line_buffer.idx);

                            display_set_footer(display, footer);
                        } else {
                            asprintf(&footer, "%s not found!", search_str);
                            display_set_footer(display, footer);
                        }
                        free(search_str);
                    }
                } break;
                }
            }
        }
        }
    }
    free(header);
    return 0;
}
// the code below allows the editor to run in a
// seperate process so that the tty mode can be restored

static size_t _lines = 20;
static int usenc = 0;

void cleanupAndExit(int sig) {
    (void)sig;
    // Just in case ncurses is acting up
    if (usenc) {
        endwin();
        system("stty sane");
    }
    exit(0);
}

typedef int (*entry)(char *);

int main(int argc, char *argv[]) {
    (void)load_extensions;
    (void)_lines;
    // Catch ctrl+c
    if (argc > 1) {
        // n for ncurses
        if (!strcmp(argv[1], "-n")) {
            usenc++;
            for (int i = 2; i < argc; i++) {
                argv[i - 1] = argv[i];
            }
            argc--;
        }
    }

    // Checking to see if the editor is being used correctly.
    if (argc != 2) {
        print_usage_error();
        return 1;
    }
    // Setting up a document based on the file named 'filename'.
    char *filename = get_filename(argc, argv);

    entry start = NULL;
    if (usenc) {
        start = ncurses_entry_point;
    } else {
        start = entry_point;
    }

    pid_t pid;
    pid = fork();
    if (pid < 0) {
        // fail
        perror("Can't fork.");
    } else if (pid == 0) {
        // child
        start(filename);
        exit(0);
    }

    signal(SIGINT, cleanupAndExit);
    int status = 0;
    waitpid(pid, &status, 0);

    cleanupAndExit(0);
}
