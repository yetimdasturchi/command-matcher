#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void similar_str(const char *txt1, int len1, const char *txt2, int len2, int *pos1, int *pos2, int *max, int *count);
int similar_char(const char *txt1, int len1, const char *txt2, int len2);