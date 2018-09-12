/**
* Extreme Edge Cases Lab
* CS 241 - Spring 2018
*/

#include "camelCaser.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

char **camel_caser(const char *input_str) {

// You can’t camelCase a NULL pointer, so if input is a NULL pointer, return a NULL pointer.
    if (!input_str) {
      return NULL;
    }
    const char *temp = input_str;
    int num_pun = 0;
    while (*temp) {
      if (ispunct(*temp)) {
        num_pun++;
      }
      temp++;
    }
    char **output_s = (char**)malloc((num_pun + 1) * sizeof(char*));
    output_s[num_pun] = NULL;
    //printf("Number of punctuation is %d\n", num_pun);
    temp = input_str;
    for (int i = 0; i < num_pun; i++) {

      const char *end = temp;
      while (end && !ispunct(*end)) {
        end++;
      }
      int length = end - temp;
      char *par_ans = malloc(length + 1);
      while (temp != end && isspace(*temp)) {
        temp++;
      }
      if (temp == end) {
        par_ans[0] = '\0';
        output_s[i] = par_ans;
        //printf("%d\n", i);
        if (temp != '\0')
          temp++;
        continue;
      }
      par_ans[0] = tolower(*temp);
      temp++;
      int loc_pa = 1;
      int flag = 0;
      while (temp != end) {

        if (isspace(*temp)) {
          flag = 1;
          temp++;
        }
        else {
          if (isalpha(*temp)) {
            if (flag) {
              flag = 0;
              par_ans[loc_pa] = toupper(*temp);
              loc_pa++;
            }
            else {
              par_ans[loc_pa] = tolower(*temp);
              loc_pa++;
            }
          }
          else {
            par_ans[loc_pa] = *temp;
            loc_pa++;
          }
          temp++;
        }
      }
      //printf("%s\n", "73");
      par_ans[loc_pa] = '\0';
      output_s[i] = par_ans;
      if (temp != '\0')
        temp++;
    }
    return output_s;
}

void destroy(char **result) {
    char **temp = result;
    while (*temp) {
      free(*temp);
      temp++;
    }
    free(result);
}
