/**
* Pointers Gone Wild Lab
* CS 241 - Spring 2018
*/

#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {
    // your code here


    int temp1 = 81;
    first_step(temp1);

    int temp2 = 132;
    second_step(&temp2);

    int temp3 = 8942;
    int *temp3_1 = &temp3;
    double_step(&temp3_1);

    int temp4 = 15;
    strange_step((char*)&temp4 - 5);

    int temp5 = 0;
    empty_step((void*)((char*)&temp5 - 3));

    char *temp6 = "value";
    two_step((void*)temp6, temp6);

    char *temp7 = "Cathy";
    three_step(temp7, temp7 + 2, temp7 + 4);

    char temp8[] = "Mark";
    temp8[2] = temp8[1] + 8;
    temp8[3] = temp8[2] + 8;
    step_step_step(temp8, temp8, temp8);

    int temp9 = 17;
    char temp9_1 = (char) temp9;
    it_may_be_odd(&temp9_1, temp9);

    char temp10[] = "I love,CS241";
    tok_step(temp10);

    int temp11 = 0x201;
    the_end((void*) &temp11, (void*) &temp11);


    return 0;
}
