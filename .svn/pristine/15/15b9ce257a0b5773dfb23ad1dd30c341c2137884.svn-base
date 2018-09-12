/**
*  Lab
* CS 241 - Spring 2018
*/

#include "utils.h"
#include <alloca.h>
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

void setup_fds(int new_stdin, int new_stdout);
void close_and_exec(char *exe, char *const *params);
pid_t start_reducer(char *reducer_exec, int in_fd, char *output_filename);
pid_t *read_input_chunked(char *filename, int *fds_to_write_to,
                          int num_mappers);
pid_t *start_mappers(char *mapper_exec, int num_mappers, int *read_mapper,
                     int write_reducer);
size_t count_lines(const char *filename);

void usage() {
    print_usage();
}

int main(int argc, char **argv) {
    // Create an input pipe for each mapper.

    // Create one input pipe for the reducer.

    // Open the output file.

    // Start a splitter process for each mapper.

    // Start all the mapper processes.

    // Start the reducer process.

    // Wait for the reducer to finish.

    // Print nonzero subprocess exit codes.

    // Count the number of lines in the output file.

    return 0;
}
