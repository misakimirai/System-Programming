/**
* Parallel Make Lab
* CS 241 - Spring 2018
*/


#include "format.h"
#include "graph.h"
#include "parmake.h"
#include "parser.h"
#include "set.h"
#include "dictionary.h"
#include "vector.h"
#include <pthread.h>

#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <stdio.h>


graph *graph_rule;
set *traversed;
set *task;
vector *subtargets;
int start_running;
pthread_mutex_t first;
pthread_mutex_t second;
pthread_cond_t first_cond;

// dfs
int det_cycle(char* node){
  set_add(traversed, node);
  vector* temp = graph_neighbors(graph_rule, node);
  for(size_t i = 0; i < vector_size(temp); i++){
    char* next = vector_get(temp, i);
    if(set_contains(traversed, next)){
      // if next is where I have already traversed then there is a cycle
      vector_destroy(temp);
      return 1;
    }
    if(det_cycle(next)){
      vector_destroy(temp);
      return 1;
    }
  }
  // backtracking
  set_remove(traversed, node);
  set_add(task, node);
  rule_t *rule = (rule_t*)graph_get_vertex_value(graph_rule, node);
  // help to check weather the dependency is fullfiled
  rule->data = (void*)graph_vertex_degree(graph_rule, node);
  vector_destroy(temp);
  return 0;
}

int run_command(char *target) {
  rule_t *curr_rule = graph_get_vertex_value(graph_rule, target);
  if (curr_rule->state != 0) {
    return curr_rule->state;
  }
  vector *dep_rules = graph_neighbors(graph_rule, target);
  // failed -> mark weather the rule is failing
  int failed = 0;
  for (size_t i = 0; i < vector_size(dep_rules); i++) {
    rule_t *temp_rule = (rule_t*)graph_get_vertex_value(graph_rule, (char*)vector_get(dep_rules, i));
    if (run_command(temp_rule->target) == -1) {
      failed = 1;
    }
  }
  if (failed) {
    curr_rule->state = -1;
    vector_destroy(dep_rules);
    return -1;
  }
  int ondisc = access(curr_rule->target, F_OK);
  int run_rule = 0;
  if (ondisc != -1) {
    struct stat time1;
    struct stat time2;
    stat(curr_rule->target, &time1);
    for (size_t i = 0; i < vector_size(dep_rules); i++) {
      char *curr_rule_dep = vector_get(dep_rules, i);
      if (access(curr_rule_dep, F_OK) == -1) {
        run_rule = 1;
        break;
      }
      stat(curr_rule_dep, &time2);
      if (difftime(time1.st_mtime, time2.st_mtime) < 0) {
        run_rule = 1;
        break;
      }
    }
    if (!run_rule) {
      curr_rule->state = 1;
      vector_destroy(dep_rules);
      return 1;
    }
  }
  vector *commands = curr_rule->commands;
  for (size_t i = 0; i < vector_size(commands); i++) {
    if (system(vector_get(commands, i)) != 0) {
      // filed = 1;
      // we need to return here for the test
      curr_rule->state = -1;
      vector_destroy(dep_rules);
      return -1;
    }
  }
  if (failed) {
    curr_rule->state = -1;
    vector_destroy(dep_rules);
    return -1;
  }
  curr_rule->state = 1;
  vector_destroy(dep_rules);
  return 1;
}


void change(char *target) {
  vector *antineighbor = graph_antineighbors(graph_rule, target);
  pthread_mutex_lock(&first);
  for (size_t i = 0; i < vector_size(antineighbor); i++) {
    char *temp = vector_get(antineighbor, i);
    rule_t *temp_rule = (rule_t*)graph_get_vertex_value(graph_rule, temp);
    temp_rule->data = (void*)(((size_t)(temp_rule->data)) - 1);
  }
  start_running = 1;
  pthread_cond_broadcast(&first_cond);
  pthread_mutex_unlock(&first);
  vector_destroy(antineighbor);
  free(target);
}

void* run_parallel(void *target) {
  while (1) {
    int finish = 0;
    char *command = NULL;
    // Now check weathere the dependency is fullfiled
    while (1) {
      pthread_mutex_lock(&first);
      start_running = 0;
      // fprintf(stderr, "%d %zu\n", __LINE__, (size_t)target);
      if (!vector_size(subtargets)) {
        // fprintf(stderr, "%d %zu\n", __LINE__, (size_t)target);
        finish = 1;
        start_running = 1;
        pthread_mutex_unlock(&first);
        break;
      }
      else {
        for (int i = vector_size(subtargets) - 1; i >= 0; i--) {
          /* though my subtargets is in topological order
             I am not going to use lisener parttern, so
             just check everything, lol
          */
          char *cur = vector_get(subtargets, i);
          rule_t *cur_rule = (rule_t*)graph_get_vertex_value(graph_rule, cur);
          if (cur_rule->data == 0) {
            // No dependency
            command = strdup(cur);
            start_running = 1;
            vector_erase(subtargets, i);
            break;
          }
        }
      }
      while (!start_running) {
        pthread_cond_wait(&first_cond, &first);
      }
      pthread_mutex_unlock(&first);
      if (command) {
        break;
      }
    }
    if (finish) {
      pthread_cond_broadcast(&first_cond);
      break;
    }
    // Now all the dependencies of the command should be none
    rule_t *command_rule = (rule_t*)graph_get_vertex_value(graph_rule, command);
    if (command_rule->state != 0) {
        change(command);
        continue;
    }
    vector *dep = graph_neighbors(graph_rule, command);
    // check for failed
    for (size_t i = 0; i < vector_size(dep); i++) {
      char *temp = vector_get(dep, i);
      rule_t *temp_rule = (rule_t*)graph_get_vertex_value(graph_rule, temp);
      if (temp_rule->state == -1) {
        command_rule->state = -1;
        break;
      }
    }
    if (command_rule->state == -1) {
      change(command);
      vector_destroy(dep);
      continue;
    }
    int run_rule = 0;
    int ondisc = access(command_rule->target, F_OK);
    if (ondisc != -1) {
      struct stat time1;
      struct stat time2;
      stat(command_rule->target, &time1);
      for (size_t i = 0; i < vector_size(dep); i++) {
        char *temp = vector_get(dep, i);
        if (access(temp, F_OK) == -1) {
          run_rule = 1;
          break;
        }
        stat(temp, &time2);
        if (difftime(time1.st_mtime, time2.st_mtime) < 0) {
          run_rule = 1;
          break;
        }
      }
      if (!run_rule) {
        command_rule->state = 1;
        vector_destroy(dep);
        change(command);
        continue;
      }
    }
    int fail = 0;
    vector *torun = command_rule->commands;
    for (size_t i = 0; i < vector_size(torun); i++) {
      if (system(vector_get(torun, i)) != 0) {
        fail = 1;
        break;
      }
    }
    if (fail) {
      command_rule->state = -1;
    }
    else {
      command_rule->state = 1;
    }
    vector_destroy(dep);
    change(command);
  }
  return NULL;
}


int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck!
    graph_rule = parser_parse_makefile(makefile, targets);
    vector *target_dep = graph_neighbors(graph_rule, "");
    for (size_t i = 0; i < vector_size(target_dep); i++) {
      traversed = string_set_create();
      task = string_set_create();
      char *target = (char*)vector_get(target_dep, i);
      if (det_cycle(target)) {
        print_cycle_failure(target);
      }
      else {
        if (num_threads == 1) {
          run_command(target);
        }
        else {
          pthread_mutex_init(&first, NULL);
          pthread_cond_init(&first_cond, NULL);
          subtargets = set_elements(task);
          pthread_t id[num_threads];
          for (size_t i = 0; i < num_threads; i++) {
              id[i] = i;
              pthread_create(id + i, NULL, run_parallel, (void*)i);
          }
          void *result[num_threads];
          for (size_t i = 0; i < num_threads; i++) {
              // fprintf(stderr, "%d %zu\n", __LINE__, i);
              pthread_join(id[i], result + i);
          }
          vector_destroy(subtargets);
          pthread_mutex_destroy(&first);
          pthread_cond_destroy(&first_cond);
        }
      }
      set_destroy(traversed);
      set_destroy(task);
    }
    graph_destroy(graph_rule);
    vector_destroy(target_dep);
    return 0;
}
