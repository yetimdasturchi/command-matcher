#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "global.h"
#include "utils.h"
#include "strtotime.h"

#define TO_DATE 447481
#define TO_NUM 89819

int words_to_date(char *input, char ** output);
int words_to_number(char* input);
char * to_macros( char * input, char * macros );