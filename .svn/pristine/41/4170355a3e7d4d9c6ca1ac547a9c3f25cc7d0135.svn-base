/**
* Text Editor Lab
* CS 241 - Spring 2018
*/

#include "sstream.h"
#include <string.h>
int main(int argc, char *argv[]) {
    // TODO cte some tests

    long i;
    bytestring wjw = {"-123",2};
    // long j = 1234567
    // printf("%ld \n", 1234567890);
    sstream *ruozhi =  sstream_create(wjw);
    sstream_parse_long(ruozhi, &i);
    printf("the position is %zu, the number is %ld \n", sstream_tell(ruozhi), i);
}
