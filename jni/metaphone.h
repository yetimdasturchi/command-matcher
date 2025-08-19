#ifndef METAPHONE_H_
#define METAPHONE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SH 'X'
#define TH '0'

void metaphone(char *word, size_t word_len, long max_phonemes, char **phoned_word, int traditional);
char* umetaphone(char *word);

#endif // METAPHONE_H_