/**
* Deadlocked Diners Lab
* CS 241 - Spring 2018
*/

#include "company.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void *work_interns(void *p) {
  Company *company = (Company*)p;
  pthread_mutex_t *left = Company_get_left_intern(company);
  pthread_mutex_t *right = Company_get_right_intern(company);
  if (left == right) {
    while(running){
      Company_have_board_meeting(company);
    }
  }
  else {
    pthread_mutex_t* temp1 = right > left ? left : right;
    pthread_mutex_t* temp2 = right > left ? right : left;
    while(running){
      pthread_mutex_lock(temp1);
      pthread_mutex_lock(temp2);
      Company_hire_interns(company);
      pthread_mutex_unlock(temp1);
      pthread_mutex_unlock(temp2);
      usleep(100);
    }
  }
  return NULL;
}
