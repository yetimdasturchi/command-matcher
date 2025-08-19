#ifndef JARO_H_
#define JARO_H_

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

double jaro_distance(char *s1, char *s2);
double jaro_winkler_distance(char *s1, char *s2, double p);

#endif // JARO_H_