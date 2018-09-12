/**
* Networking Lab
* CS 241 - Spring 2018
*/

#include "common.h"
#include "format.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


char **parse_args(int argc, char **argv);
verb check_args(char **args);


int main(int argc, char **argv) {
    // Good luck!
    check_args(argv);
    char **commands = parse_args(argc, argv);


    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);

  	struct addrinfo hints, *result;
  	memset(&hints, 0, sizeof(struct addrinfo));
  	hints.ai_family = AF_INET; /* IPv4 only */
  	hints.ai_socktype = SOCK_STREAM; /* TCP */

  	int s = getaddrinfo(commands[0], commands[1], &hints, &result);
  	if (s != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
    	exit(1);
  	}

  	if(connect(sock_fd, result->ai_addr, result->ai_addrlen) == -1){
      perror("connect");
      exit(2);
    }

    char buffer[1024];
    if (!strcmp(commands[2],"GET")) {
      sprintf(buffer, "GET %s\n", commands[3]);
      write_all_to_socket(sock_fd, buffer, strlen(buffer));
    }
    else if (!strcmp(commands[2], "PUT")) {
      struct stat temp;
      stat(commands[4], &temp);
      int file = open(commands[4], O_RDONLY, 0666);
      if (file < 0) {
        printf("No such file or directory\n");
        free(commands);
        freeaddrinfo(result);
        exit(1);
      }
      size_t size = temp.st_size;
      // printf("The size here is %zu, the line is %d\n", size, __LINE__);
      sprintf(buffer, "PUT %s\n", commands[3]);
      write_all_to_socket(sock_fd, buffer, strlen(buffer));
      write_message_size((uint64_t)size, sock_fd);
      // int binary_size = 2333;
      // char binary_data[binary_size];
      // ssize_t sent = 0;
      // for (; (int)size > 0; size -= binary_size) {
      //   if ((int)size > binary_size) {
      //     sent += read_all_from_socket(file, binary_data, binary_size);
      //     write_all_to_socket(sock_fd, binary_data, binary_size);
      //   }
      //   else {
      //     sent += read_all_from_socket(file, binary_data, size);
      //     write_all_to_socket(sock_fd, binary_data, size);
      //   }
      // }
      char *buff = calloc(1, 65536);
      ssize_t length = 0;
      while ((length = read_all_from_socket(file, buff, 65535)) > 0) {
          write_all_to_socket(sock_fd, buff, length);
      }
      free(buff);
      close(file);
    }
    else if (!strcmp(commands[2], "DELETE")) {
      sprintf(buffer, "DELETE %s\n", commands[3]);
      write_all_to_socket(sock_fd, buffer, strlen(buffer));
    }
    else {
      // "LIST"
      sprintf(buffer, "LIST\n");
      write_all_to_socket(sock_fd, buffer, strlen(buffer));
    }

    shutdown(sock_fd, SHUT_WR);




    // Response

    char response[10];
    char error_message[1000];
    read_all_from_socket(sock_fd, response, 3);
    response[3] = '\0';
    if (!strcmp(response, "OK\n"))  {
      // success
      if (!strcmp(commands[2], "PUT") || !strcmp(commands[2], "DELETE")) {
        // We are done!
        print_success();
        free(commands);
        freeaddrinfo(result);
        exit(0);
      }

      // consider get and list
      int total = get_message_size(sock_fd);
      // int compare = total;
      // printf("The totla number of things received is %d, the line is %d\n", total, __LINE__);
      if (!strcmp(commands[2], "GET")) {
        int destination = open(commands[4], O_CREAT | O_TRUNC | O_RDWR, 0666);

        // int binary_size = 2333;
        // char binary_data[binary_size];
        // ssize_t sent = 0;
        // for (; total > 0; total -= binary_size) {
        //   if (total > binary_size) {
        //     sent += read_all_from_socket(sock_fd, binary_data, binary_size);
        //     write_all_to_socket(destination, binary_data, binary_size);
        //   }
        //   else {
        //     sent += read_all_from_socket(sock_fd, binary_data, total);
        //     write_all_to_socket(destination, binary_data, total);
        //   }
        // }
        // // printf("The number of things actually received is %zd, the line is %d\n", sent, __LINE__);
        // if (sent < compare) {
        //   print_too_little_data();
        // }
        // else {
        //   if (read_all_from_socket(sock_fd, binary_data, 1) > 0) {
        //     // I can read more than total
        //     print_received_too_much_data();
        //   }
        // }
        char *buff = calloc(1, 65536);
        ssize_t length = 0;
        ssize_t partial = 0;
        while ((partial = read_all_from_socket(sock_fd, buff, 65535))) {
            write_all_to_socket(destination, buff, partial);
            length += partial;
        }
        if (length < total) {
            print_too_little_data();
            return 1;
        }
        if (length > total) {
            print_received_too_much_data();
            return 1;
        }
        free(buff);
      }
      else {
        // LIST
        // int binary_size = 1024;
        // char binary_data[binary_size];
        // ssize_t sent = 0;
        // for (; total > 0; total -= binary_size) {
        //   if (total > binary_size) {
        //     sent += read_all_from_socket(sock_fd, binary_data, binary_size);
        //     write_all_to_socket(1, binary_data, binary_size);
        //   }
        //   else {
        //     sent += read_all_from_socket(sock_fd, binary_data, total);
        //     write_all_to_socket(1, binary_data, total);
        //   }
        // }
        // if (sent < compare) {
        //   print_too_little_data();
        // }
        // else {
        //   if (read_all_from_socket(sock_fd, binary_data, 1) > 0) {
        //     // I can read more than total
        //     print_received_too_much_data();
        //   }
        // }
        char *buff = calloc(1, 65536);
        ssize_t length = 0;
        ssize_t partial = 0;
        while ((partial = read_all_from_socket(sock_fd, buff, 65535))) {
            write_all_to_socket(1, buff, partial);
            length += partial;
        }
        if (length < total) {
            print_too_little_data();
            return 1;
        }
        if (length > total) {
            print_received_too_much_data();
            return 1;
        }
        free(buff);
      }
      free(commands);
      freeaddrinfo(result);
      exit(0);
    }
    else {
      // There are errors, first read the left message in first line
      read_all_from_socket(sock_fd, response, 3);
      // Now read the error MESSAGE
      int temp = 0;
      for (int count = 0; count < 1000; count += temp) {
        temp = read_all_from_socket(sock_fd, error_message + count, 1);
        if (error_message[count] == '\n') {
          error_message[count] = '\0';
          // print_error_message("Bellow is the error message\n");
          print_error_message(error_message);
          // print_error_message("Above is the error message\n");
          free(commands);
          freeaddrinfo(result);
          exit(1);
        }
      }
    }
    return 0;



}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }

    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(1);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(1);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(1);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(1);
}
