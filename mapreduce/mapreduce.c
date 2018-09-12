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

    // Start all the mapper splitter_pid.

    // Start the reducer process.

    // Wait for the reducer to finish.

    // Print nonzero subprocess exit codes.

    // Count the number of lines in the output file.

    if (argc != 6) {
      print_usage();
      exit(1);
    }
    char *input = argv[1];
    int count = atoi(argv[argc-1]);
    int mapper_pipe[count][2];
    // for (int i = 0; i < count; i++) {
    //   pipe(mapper_pipe[i]);
    // }
    pid_t splitter_pid[count];
    for (int i = 0; i < count; i++) {
      pipe(mapper_pipe[i]);
      splitter_pid[i] = fork();
      if (splitter_pid[i]) {
        // I am parent
        close(mapper_pipe[i][1]);
        // close(mapper_pipe[i][0]);
      }
      else {
        // I am child
        close(mapper_pipe[i][0]);
        dup2(mapper_pipe[i][1], 1);
        char first[10];
        char second[10];
        // itoa(count, first, 10);
        // itoa(i, second, 10);
        sprintf(first, "%d", count);
        sprintf(second, "%d", i);
        execlp("./splitter", "./splitter", input, first, second, (char*)NULL);
  			fprintf(stderr, "Failed in splitter\n");
  			exit(-1);
      }
    }



    int reducer_pipe[2];
  	pipe(reducer_pipe);

    pid_t mapper_pid[count];
    for (int i = 0; i < count; i++) {
      mapper_pid[i] = fork();
      if (mapper_pid[i]) {
        // I am parent
        // close(mapper_pipe[i][1]);
  			close(mapper_pipe[i][0]);
      }
      else {
        // close(mapper_pipe[i][1]);
        close(reducer_pipe[0]);
        dup2(mapper_pipe[i][0], 0);
        dup2(reducer_pipe[1], 1);
        execlp(argv[3], argv[3], NULL);
        fprintf(stderr, "Failed in mapper\n");
  			exit(-1);
      }
    }




    int reducer_status;
    int output = open(argv[2], O_CREAT | O_TRUNC | O_RDWR, 0666);
    pid_t reducer_pid = fork();
    if (!reducer_pid) {
      // I am child
      close(reducer_pipe[1]);
      dup2(reducer_pipe[0], 0);
      // int output = open(argv[2], O_CREAT | O_TRUNC | O_RDWR, 0666);
      dup2(output, 1);
      execlp(argv[4], argv[4], NULL);
      fprintf(stderr, "The reducer part failed.\n");
  		exit(-1);
    }
    // I am parent

    close(reducer_pipe[0]);
    close(reducer_pipe[1]);

    int splitter_status[count];
    for (int i = 0; i < count; i++) {
      waitpid(splitter_pid[i], splitter_status + i, 0);
    }

    int mapper_status[count];
    for (int i = 0; i < count; i++) {
      waitpid(splitter_pid[i], mapper_status + i, 0);
    }

    waitpid(reducer_pid, &reducer_status, 0);

    close(output);
    // close(input);
    print_num_lines(argv[2]);


    for (int i = 0; i < count; i++) {
      if (WIFEXITED(splitter_status[i]) && WEXITSTATUS(splitter_status[i])) {
        print_nonzero_exit_status("./splitter", WEXITSTATUS(splitter_status[i]));
      }
    }

    for (int i = 0; i < count; i++) {
      if (WIFEXITED(mapper_status[i]) && WEXITSTATUS(mapper_status[i])) {
        print_nonzero_exit_status(argv[3], WEXITSTATUS(mapper_status[i]));
      }
    }

    // int reducer_status;
    waitpid(reducer_pid, &reducer_status, 0);
    if (WIFEXITED(reducer_status) && WEXITSTATUS(reducer_status)) {
      print_nonzero_exit_status(argv[4], WEXITSTATUS(reducer_status));
    }



    return 0;
}
