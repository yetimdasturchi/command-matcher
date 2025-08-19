#ifndef GLOBAL_H_
#define GLOBAL_H_

#define type_name(x) _Generic((x), \
    char: "char", \
    unsigned char: "unsigned_char", \
    signed char: "signed_char", \
    short: "short", \
    unsigned short: "unsigned_short", \
    int: "int", \
    unsigned int: "unsigned_int", \
    long: "long", \
    unsigned long: "unsigned_long", \
    long long: "long_long", \
    unsigned long long: "unsigned_long_long", \
    float: "float", \
    double: "double", \
    long double: "long_double", \
    char *: "char_pointer", \
    const char *: "const_char_pointer", \
    default: "unknown")

#define MAX_LEN 150

typedef struct {
    char text[100];
    int id;
} Command;

typedef struct {
    char *domain;
    char *operational_label;
    char *original;
    char *answer;
    char *json;
    char *regex;
} SingleCommand;

typedef struct {
    char *result;    
} RegexMatches;

typedef struct {
    int group;
    char *key;
    int value;
    char *macros;
} MacrosMatches;

typedef struct {
    unsigned int hash;
    int similarity;
} SimilarityCache;

extern Command* commands;
extern SingleCommand* command;
extern int commands_size;

extern SimilarityCache* similarity_cache;
extern int similarity_cache_size;

extern char global_keyword[100];
extern char * res_path;

static struct { char *key; char *value; } str_to_time_dictionary[] = {
    {"plus", "+"},
    {"minus", "-"},
    {"bir", "1"},
    {"ikki", "2"},
    {"uch", "3"},
    {"tort", "4"},
    {"besh", "5"},
    {"kun", "day"},
    {"erta", "+1 day"},
    {"bugun", "today"},
    {"indin", "+2 day"},
    {"kecha", "-1 day"},
    {"dushanba", "monday"},
    {"seshanba", "tuesday"},
    {"chorshanba", "wednesday"},
    {"payshanba", "thursday"},
    {"juma", "friday"},
    {"shanba", "saturday"},
    {"yakshanba", "sunday"},
    {"kechagi", "last"},
    {"avvalgi", "last"},
    {"keyingi", "next"},
    {NULL, NULL}
};

static struct { char* word; int value; } word_to_number_dictionary[] = {
    {"nl", 0},
    {"br", 1},
    {"ik", 2},
    {"ux", 3},
    {"trt", 4},
    {"bx", 5},
    {"olt", 6},
    {"yt", 7},
    {"sks", 8},
    {"tks", 9},
    {"on", 10},
    {"onbr", 11},
    {"onk", 12},
    {"onx", 13},
    {"ontrt", 14},
    {"onbx", 15},
    {"onlt", 16},
    {"onyt", 17},
    {"onsks", 18},
    {"ontks", 19},
    {"ykrm", 20},
    {"ots", 30},
    {"krk", 40},
    {"elk", 50},
    {"oltmx", 60},
    {"ytmx", 70},
    {"sksn", 80},
    {"tksn", 90},
    {NULL, 0}
};

#endif // GLOBAL_H_