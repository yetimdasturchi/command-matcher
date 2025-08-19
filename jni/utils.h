#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "global.h"
#include "metaphone.h"
#include "similarity.h"
#include "cJSON.h"
#include "re.h"
#include "jaro.h"

char** split_and_metaphone(const char* input, int* count);
char** split_to_words(const char* input, int* count);
int load_commands();
void init_smilarity_cache(int size);
void free_smilarity_cache();
void clean_string(const char *input, char *output);
void simplify_string(char* str);
int compare_commands(const void *a, const void *b);
int split(const char* str, char words[][MAX_LEN]);
int levenshtein_distance(char *s1, char *s2);
void set_nested_value(cJSON *json, const char *path, const char *value);
char* remove_duplicate_words(char* str);

char* str_to_lower(char* s);
char* str_to_upper(char* s);

int find_matches(const char* str, const char* pattern, RegexMatches** outMatches);
char* str_replace(const char* subject, const char* search, const char* replace);

char* remove_substring(char *str, const char *toRemove);

int init_database();
void close_database();

SingleCommand* get_single_command( int id );
void free_single_command(SingleCommand* command);
void fetch_macros_matches(MacrosMatches** matches, int* count, int command);

char *concat(char *a, char *b);
char* trim( char* str, const char ch, int mode );
int is_directory_exists(const char *path);

Command *sort_database(Command *database, int n, char *keyword);

void replace_white_space(char *str);
double match(char *s1, char *s2, bool comparePhonetics);