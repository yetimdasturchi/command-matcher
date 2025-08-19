#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

void convert_epoch_to_human_readable(time_t epochTime, char ** result);
time_t string_to_date(char *input);