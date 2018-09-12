/**
* Extreme Edge Cases Lab
* CS 241 - Spring 2018
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

/*
 * Testing function for various implementations of camelCaser.
 *
 * @param  camelCaser   A pointer to the target camelCaser function.
 * @param  destroy      A pointer to the function that destroys camelCaser
 * output.
 * @return              Correctness of the program (0 for wrong, 1 for correct).
 */
int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    // TODO: Return 1 if the passed in function works properly; 0 if it doesn't.

    // =========================================================================
    char **result1 = camelCaser("  TrueOrFaSE  ..?  !   132 cmt.5dFe 6DfE.5DFE 6dfE CMTD.  FoO\nBar. foo\nbar  . what do");
    if(*result1 == NULL || *(result1+1) == NULL || *(result1+2) == NULL || *(result1+3) == NULL ||
        *(result1+4) == NULL || *(result1+5) == NULL || *(result1+6) == NULL || *(result1+7) == NULL ||
        *(result1+8) == NULL || *(result1+9) != NULL) {
          return 0;
        }
    if(strcmp(*result1, "trueorfase") != 0)
      return 0;
    if(strlen(*(result1+1)) != 0)
      return 0;
    if(strlen(*(result1+2)) != 0)
      return 0;
    if(strlen(*(result1+3)) != 0)
      return 0;
    if(strcmp(*(result1+4), "132Cmt") != 0)
      return 0;
    if(strcmp(*(result1+5), "5dfe6Dfe") != 0)
      return 0;
    if(strcmp(*(result1+6), "5dfe6DfeCmtd") != 0)
      return 0;
    if(strcmp(*(result1+7), "fooBar") != 0)
      return 0;
    if(strcmp(*(result1+8), "fooBar") != 0)
      return 0;

    char **result2 = camelCaser("");
    char **result3 = camelCaser("what do you want");
    char **result4 = camelCaser("   ");
    if (*result2 != NULL || *result3 != NULL || *result4 != NULL) {
      return 0;
    }

    char **result5 = camelCaser("1234567890:");
    if (*result5 == NULL || *(result5 + 1) != NULL) {
      return 0;
    }
    if (strcmp(*result5, "1234567890") != 0) {
      return 0;
    }

    char **result6 = camelCaser("567 8Df 85 MP?567 8dF9 476 mp / for\tloOpS5H6y.    RHSAOfdso RIsjGIShhs8 sd89hw9odshf oowjdDsoAIDog.  lONGsSdgho dishousd\n oidhosd \nshdogshdognSodhgSYOSd37sd8ghs.dishog782D 7dg 9sh 738ET89fjid.");
    if (*result6 == NULL || *(result6+1) == NULL || *(result6+2) == NULL || *(result6+3) == NULL
        || *(result6+4) == NULL || *(result6+5) == NULL || *(result6+6) != NULL)
      return 0;
    if(strcmp(result6[0], "5678Df85Mp") != 0) return 0;
    if(strcmp(result6[1], "5678Df9476Mp") != 0) return 0;
    if(strcmp(result6[2], "forLoops5h6y") != 0) return 0;
    if(strcmp(result6[3], "rhsaofdsoRisjgishhs8Sd89hw9odshfOowjddsoaidog") != 0) return 0;
    if(strcmp(result6[4], "longssdghoDishousdOidhosdShdogshdognsodhgsyosd37sd8ghs") != 0) return 0;
    if(strcmp(result6[5], "dishog782d7Dg9Sh738Et89fjid") != 0) return 0;

    destroy(result1);
    destroy(result2);
    destroy(result3);
    destroy(result4);
    destroy(result5);
    destroy(result6);

    return 1;
}
