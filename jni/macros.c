#include "macros.h"

char *translate_time_word(char *word) {
    int shortest = -1;
    char *closestWord = NULL;
    for (int i = 0; str_to_time_dictionary[i].key != NULL; i++) {
        int lev = levenshtein_distance(word, str_to_time_dictionary[i].key);
        if (lev == 0) {
            closestWord = str_to_time_dictionary[i].value;
            break;
        }
        if (lev < shortest || shortest < 0) {
            closestWord = str_to_time_dictionary[i].value;
            shortest = lev;
        }
    }
    return closestWord;
}

void translate_time_phrase(char *input, char ** output) {
    char *word = strtok(input, " ");
    char translatedPhrase[100] = "";

    while (word != NULL) {
        char *translation = translate_time_word(word);
        if (translation != NULL) {
            strcat(translatedPhrase, translation);
            strcat(translatedPhrase, " ");
        }
        word = strtok(NULL, " ");
    }

    *output = (char *)realloc( *output, strlen(translatedPhrase));

    if (*output == NULL) {
        printf("Memory allocation failed\n");
        return;
    }

    strcat( *output, translatedPhrase );
}

int words_to_date(char *input, char ** output){
    char * phrase = NULL;
    translate_time_phrase(input, &phrase);
    
    if ( phrase == NULL ) {
        return -1;
    }

    time_t result = string_to_date( phrase );

    if(result != -1) {
        convert_epoch_to_human_readable(result, output);
        return 1;
    }

    return -1;
};


/* words to int */

int lookup_word(const char* word) {
    for (int i = 0; word_to_number_dictionary[i].word != NULL; i++) {
        if (strcmp(word_to_number_dictionary[i].word, word) == 0) {
            return word_to_number_dictionary[i].value;
        }
    }
    return -1;
}

int process_words(char* words) {
    int number = 0;
    int current = 0;
    char* word = strtok(words, " ");
    while (word) {
        word = remove_substring(word, "ka");
        word = remove_substring(word, "ga");
        char *meta_word = NULL;
        metaphone(word, strlen(word), 0, &meta_word, 1);
        meta_word = str_to_lower( meta_word );
        int value = lookup_word(meta_word);
        if (value != -1) {
            current += value;
        } else if (strcmp(meta_word, "ys") == 0) {
            current *= 100;
        } else if (strcmp(meta_word, "mnk") == 0) {
            number += current * 1000;
            current = 0;
        } else if (strcmp(meta_word, "mln") == 0) {
            number += current * 1000000;
            current = 0;
        }
        free( meta_word );
        word = strtok(NULL, " ");
    }
    number += current;
    return number;
}

int words_to_number(char* input) {
    char buffer[1024];
    strncpy(buffer, input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';

    for (int i = 0; buffer[i]; i++) {
        buffer[i] = tolower(buffer[i]);
        if (buffer[i] == '-') {
            buffer[i] = ' ';
        }
    }

    return process_words(buffer);
}

int hash_name(const char *str) {
    int hash = 0;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 2) + hash) + c;
    }

    return hash;
}

char * to_macros( char * input, char * macros ) {
    switch( hash_name( macros ) ) {
        case TO_DATE: {
            char * output = NULL;
            int status = words_to_date(input, &output);
            
            if ( status == -1 ) {
                return NULL;
            }

            return output;
        }

        case TO_NUM: {
            int number = words_to_number( input );
            int length = snprintf(NULL, 0, "%d", number);
            char *str = (char *)malloc(length + 1);

            if (str != NULL) {
                snprintf(str, length + 1, "%d", number);
                return str;
            } else {
                return "0";
            }
        }

        default:
            break;
    }

    return input;
}