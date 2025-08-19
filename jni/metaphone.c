#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define SH 'X'
#define TH '0'

void metaphone(char *word, size_t word_len, long max_phonemes, char **phoned_word, int traditional);

int is_vowel(char c) {
    c = toupper(c);
    return (c == 'A' || c == 'E' || c == 'I' || c == 'O' || c == 'U');
}

int affecth(char c) {
    c = toupper(c);
    return (c != 'C' && c != 'G' && c != 'P' && c != 'S' && c != 'T');
}

char look_back_letter(char *word, int n) {
    int idx = n;
    while (word[idx] != '\0' && idx >= 0) {
        idx--;
    }
    return (idx >= 0) ? toupper(word[idx]) : '\0';
}

char look_ahead_letter(char *word, int n) {
    int idx = 0;
    while (word[idx] != '\0' && idx < n) {
        idx++;
    }
    return toupper(word[idx]);
}

void metaphone(char *word, size_t word_len, long max_phonemes, char **phoned_word, int traditional) {
    int w_idx = 0;
    size_t p_idx = 0;
    size_t max_buffer_len = 0;
    char curr_letter;

    if (max_phonemes == 0) {
        max_buffer_len = word_len;
        *phoned_word = (char *)malloc(sizeof(char) * (word_len + 1));
    } else {
        max_buffer_len = max_phonemes;
        *phoned_word = (char *)malloc(sizeof(char) * (max_phonemes + 1));
    }

    if (*phoned_word == NULL) {
        printf("Memory allocation failed\n");
        return;
    }

    for (; !isalpha(curr_letter = toupper(word[w_idx])); w_idx++) {
        if (curr_letter == '\0') {
            (*phoned_word)[p_idx] = '\0';
            return;
        }
    }

    switch (curr_letter) {
        case 'A':
            if (toupper(word[w_idx + 1]) == 'E') {
                (*phoned_word)[p_idx++] = 'E';
                w_idx += 2;
            } else {
                (*phoned_word)[p_idx++] = 'A';
                w_idx++;
            }
            break;
        case 'G':
        case 'K':
        case 'P':
            if (toupper(word[w_idx + 1]) == 'N') {
                (*phoned_word)[p_idx++] = 'N';
                w_idx += 2;
            }
            break;
        case 'W':
            if (toupper(word[w_idx + 1]) == 'R') {
                (*phoned_word)[p_idx++] = 'R';
                w_idx += 2;
            } else if (toupper(word[w_idx + 1]) == 'H' || is_vowel(toupper(word[w_idx + 1]))) {
                (*phoned_word)[p_idx++] = 'W';
                w_idx += 2;
            }
            break;
        case 'X':
            (*phoned_word)[p_idx++] = 'S';
            w_idx++;
            break;
        case 'E':
        case 'I':
        case 'O':
        case 'U':
            (*phoned_word)[p_idx++] = curr_letter;
            w_idx++;
            break;
        default:
            break;
    }

    for (; (curr_letter = toupper(word[w_idx])) != '\0' && (max_phonemes == 0 || p_idx < (size_t)max_phonemes); w_idx++) {
        unsigned short int skip_letter = 0;

        if (!isalpha(curr_letter))
            continue;

        char prev_letter = (w_idx >= 1) ? toupper(word[w_idx - 1]) : '\0';

        if (curr_letter == prev_letter && curr_letter != 'C')
            continue;

        switch (curr_letter) {
            case 'B':
                if (prev_letter != 'M')
                    (*phoned_word)[p_idx++] = 'B';
                break;
            case 'C':
                if (toupper(word[w_idx + 1]) == 'H') {
                    if ((!traditional) && (prev_letter == 'S' || toupper(word[w_idx + 2]) == 'R'))
                        (*phoned_word)[p_idx++] = 'K';
                    else
                        (*phoned_word)[p_idx++] = SH;
                    skip_letter++;
                } else if (toupper(word[w_idx + 1]) == 'I' && (toupper(word[w_idx + 2]) == 'A' || toupper(word[w_idx + 2]) == 'E' || toupper(word[w_idx + 2]) == 'Y')) {
                    (*phoned_word)[p_idx++] = SH;
                } else if (!traditional && toupper(word[w_idx + 1]) == 'C' && toupper(word[w_idx + 2]) == 'H' && toupper(word[w_idx + 3]) == 'W') {
                    (*phoned_word)[p_idx++] = SH;
                    skip_letter += 2;
                } else {
                    (*phoned_word)[p_idx++] = 'K';
                }
                break;
            case 'D':
                if (toupper(word[w_idx + 1]) == 'G' && (toupper(word[w_idx + 2]) == 'I' || toupper(word[w_idx + 2]) == 'E' || toupper(word[w_idx + 2]) == 'Y')) {
                    (*phoned_word)[p_idx++] = 'J';
                    skip_letter++;
                } else {
                    (*phoned_word)[p_idx++] = 'T';
                }
                break;
            case 'G':
                if (toupper(word[w_idx + 1]) == 'H') {
                    if (!(toupper(word[w_idx - 3]) == 'B' || toupper(word[w_idx - 2]) == 'D' || toupper(word[w_idx - 2]) == 'H' || toupper(word[w_idx - 1]) == 'H')) {
                        (*phoned_word)[p_idx++] = 'F';
                        skip_letter++;
                    }
                } else if (toupper(word[w_idx + 1]) == 'N' && (isalpha(toupper(word[w_idx + 2])) && !isalpha(toupper(word[w_idx + 3])))) {
                    (*phoned_word)[p_idx++] = 'K';
                } else if (toupper(word[w_idx + 1]) == 'E' && (toupper(word[w_idx + 2]) == 'Y' || toupper(word[w_idx + 2]) == 'I' || toupper(word[w_idx + 2]) == 'E')) {
                    if (toupper(word[w_idx - 1]) != 'G')
                        (*phoned_word)[p_idx++] = 'J';
                } else {
                    (*phoned_word)[p_idx++] = 'K';
                }
                break;
            case 'H':
                if (is_vowel(toupper(word[w_idx + 1])) && !affecth(prev_letter))
                    (*phoned_word)[p_idx++] = 'H';
                break;
            case 'K':
                if (toupper(word[w_idx - 1]) != 'C')
                    (*phoned_word)[p_idx++] = 'K';
                break;
            case 'P':
                if (toupper(word[w_idx + 1]) == 'H')
                    (*phoned_word
                    )[p_idx++] = 'F';
                else
                    (*phoned_word)[p_idx++] = 'P';
                break;
            case 'Q':
                (*phoned_word)[p_idx++] = 'K';
                break;
            case 'S':
                if (toupper(word[w_idx + 1]) == 'I' && (toupper(word[w_idx + 2]) == 'O' || toupper(word[w_idx + 2]) == 'A'))
                    (*phoned_word)[p_idx++] = SH;
                else if (toupper(word[w_idx + 1]) == 'H')
                    (*phoned_word)[p_idx++] = SH;
                else if (!traditional && toupper(word[w_idx + 1]) == 'C' && toupper(word[w_idx + 2]) == 'H' && toupper(word[w_idx + 3]) == 'W')
                    (*phoned_word)[p_idx++] = SH;
                else
                    (*phoned_word)[p_idx++] = 'S';
                break;
            case 'T':
                if (toupper(word[w_idx + 1]) == 'I' && (toupper(word[w_idx + 2]) == 'O' || toupper(word[w_idx + 2]) == 'A'))
                    (*phoned_word)[p_idx++] = SH;
                else if (toupper(word[w_idx + 1]) == 'H')
                    (*phoned_word)[p_idx++] = TH;
                else if (!(toupper(word[w_idx + 1]) == 'C' && toupper(word[w_idx + 2]) == 'H'))
                    (*phoned_word)[p_idx++] = 'T';
                break;
            case 'V':
                (*phoned_word)[p_idx++] = 'F';
                break;
            case 'W':
                if (is_vowel(toupper(word[w_idx + 1])))
                    (*phoned_word)[p_idx++] = 'W';
                break;
            case 'X':
                (*phoned_word)[p_idx++] = 'K';
                (*phoned_word)[p_idx++] = 'S';
                break;
            case 'Y':
                if (is_vowel(toupper(word[w_idx + 1])))
                    (*phoned_word)[p_idx++] = 'Y';
                break;
            case 'Z':
                (*phoned_word)[p_idx++] = 'S';
                break;
            case 'F':
            case 'J':
            case 'L':
            case 'M':
            case 'N':
            case 'R':
                (*phoned_word)[p_idx++] = curr_letter;
                break;
            default:
                break;
        }

        w_idx += skip_letter;
    }

    (*phoned_word)[p_idx] = '\0';
}

char* umetaphone(char *word) {
    char *s = strdup(word);
    int len = strlen(s);
    int pos = 0;

    for (int i = 0; i < len; i++) {
        s[i] = toupper(s[i]);
    }

    char * result = malloc((len * 2 + 1) * sizeof(char));

    if (result == NULL) {
        return NULL;
    }

    int resultPos = 0;

    while (pos < len) {
        char c = s[pos];
        char next = pos < len - 1 ? s[pos + 1] : '\0';
        char nextnext = pos < len - 2 ? s[pos + 2] : '\0';

        switch (c) {
            case 'C':
                if (next == 'H') {
                    result[resultPos++] = 'J';
                    pos++;
                } else {
                    result[resultPos++] = 'S';
                }
                break;

            case 'A':
                if (pos == 0 || pos == len - 1) {
                    result[resultPos++] = 'A';
                } else if (next == 'O') {
                    result[resultPos++] = 'O';
                    pos++;
                } else if (next == 'I') {
                    result[resultPos++] = 'Y';
                    pos++;
                }
                break;

            case 'E':
                if (pos == 0 || pos == len - 1) {
                    result[resultPos++] = 'I';
                }
                break;

            case 'I':
                if (pos == 0) {
                    result[resultPos++] = 'I';
                } else if (next == 'A' || next == 'O') {
                    result[resultPos++] = 'Y';
                    pos++;
                }
                break;

            case 'O':
                if (pos == 0) {
                    if (next == '\'') {
                        result[resultPos++] = 'U';
                        pos++;
                    } else if (next == 'I') {
                        result[resultPos++] = 'O';
                        result[resultPos++] = 'Y';
                        pos++;
                    } else {
                        result[resultPos++] = 'O';
                    }
                } else if (next == 'I') {
                    result[resultPos++] = 'Y';
                    pos++;
                } else if (pos == len - 1) {
                    result[resultPos++] = 'O';
                }
                break;

            case 'B':
            case 'P':
                result[resultPos++] = 'P';
                if (next == c) pos++;
                break;

            case 'F':
            case 'V':
                result[resultPos++] = 'F';
                if (next == c) pos++;
                break;

            case 'D':
                result[resultPos++] = 'T';
                if (next == c) pos++;
                break;

            case 'Q':
            case 'K':
                result[resultPos++] = 'K';
                if (next == c) pos++;
                break;

            case 'G':
                result[resultPos++] = 'K';
                if (next == '\'') {
                    pos++;
                } else if (next == c) {
                    pos++;
                }
                break;

            case 'H':
            case 'X':
                result[resultPos++] = 'X';
                if (next == 'T' && !is_vowel(nextnext)) {
                    pos++;
                } else if (next == c) {
                    pos++;
                }
                break;

            case 'J':
                result[resultPos++] = 'J';
                if (next == c) pos++;
                break;

            case 'L':
            case 'M':
            case 'N':
            case 'R':
                result[resultPos++] = c;
                if (next == c) pos++;
                break;

            case 'S':
                result[resultPos++] = 'S';
                if (next == 'H' || next == c) pos++;
                break;

            case 'T':
                if (next == 'S') {
                    result[resultPos++] = 'S';
                    pos++;
                } else {
                    result[resultPos++] = 'T';
                    if (next == c) pos++;
                }
                break;

            case 'U':
                if (pos == 0 || pos == len - 1) {
                    result[resultPos++] = 'U';
                }
                break;

            case 'Y':
                if (pos > 0 && (next == 'A' || next == 'U' || next == 'O' || next == 'E')) {
                    result[resultPos++] = next;
                    pos++;
                } else {
                    result[resultPos++] = 'Y';
                    if (next == c) pos++;
                }
                break;

            case 'Z':
                result[resultPos++] = 'S';
                if (next == c) pos++;
                break;

            case ' ':
                result[resultPos++] = ' ';
                if (next == c) pos++;
                break;
        }
        pos++;
    }

    result[resultPos] = '\0';
    return result;
}