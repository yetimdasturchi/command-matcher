#include "utils.h"
#include "sqlite3.h"

sqlite3 *db;
int rc;

int init_database() {
    rc = sqlite3_open( concat(res_path, "index.db"), &db);
    if (rc != SQLITE_OK) {
        sqlite3_close(db);

        return -1;
    }

    return 0;
}

void close_database() {
    sqlite3_close(db);
}

Command* add_commands_index(Command* commands, int* commands_size) {
    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, "SELECT command, text FROM ss;", -1, &stmt, 0);

    if (rc == SQLITE_OK) {
        Command temp;
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            int command = sqlite3_column_int(stmt, 0);
            const unsigned char *text = sqlite3_column_text(stmt, 1);

            Command temp;
            temp.id = command;
            strncpy(temp.text, (const char *)text, sizeof(temp.text) - 1);
            temp.text[sizeof(temp.text) - 1] = '\0';
            
            Command* resized = realloc(commands, (*commands_size + 1) * sizeof(Command));
            if (resized == NULL) {
                break;
            }

            commands = resized;
            commands[*commands_size] = temp;
            (*commands_size)++;
        }
        
        sqlite3_finalize(stmt);
    } else {
        sqlite3_close(db);
    }

    return commands;
}

int load_commands() {
	commands = malloc( sizeof( Command ) );
    
    if (commands == NULL) {
        return -1;
    }

    commands = add_commands_index(commands, &commands_size);

    return 0;
}

SingleCommand* get_single_command( int id ) {
    sqlite3_stmt *stmt;
    SingleCommand* command = NULL;

    const char *sql = "SELECT domain, operational_label, original, answer, json, regex FROM list WHERE command = ?";
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        return NULL;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        command = malloc(sizeof(SingleCommand));
        if (command == NULL) {
            sqlite3_finalize(stmt);
            return NULL;
        }

        command->domain = strdup((const char*)sqlite3_column_text(stmt, 0));
        command->operational_label = strdup((const char*)sqlite3_column_text(stmt, 1));
        command->original = strdup((const char*)sqlite3_column_text(stmt, 2));
        command->answer = strdup((const char*)sqlite3_column_text(stmt, 3));
        command->json = strdup((const char*)sqlite3_column_text(stmt, 4));
        command->regex = strdup((const char*)sqlite3_column_text(stmt, 5));
    }

    sqlite3_finalize(stmt);
    return command;
}

void fetch_macros_matches(MacrosMatches** matches, int* count, int command) {
    const char *sql = "SELECT key, value, macros FROM matches WHERE command = ?;";
    sqlite3_stmt *stmt;
    int rc, capacity = 10;

    *count = 0;
    *matches = malloc(capacity * sizeof(MacrosMatches));
    if (!*matches) {
        return;
    }

    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        free(*matches);
        *matches = NULL;
        return;
    }

    sqlite3_bind_int(stmt, 1, command);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        if (*count >= capacity) {
            capacity *= 2;
            MacrosMatches* temp = realloc(*matches, capacity * sizeof(MacrosMatches));
            if (!temp) {
                break;
            }
            *matches = temp;
        }

        (*matches)[*count].key = strdup((const char *)sqlite3_column_text(stmt, 0));
        (*matches)[*count].value = sqlite3_column_int(stmt, 1);
        (*matches)[*count].macros = strdup((const char *)sqlite3_column_text(stmt, 2));

        (*count)++;
    }

    sqlite3_finalize(stmt);
}


void free_single_command(SingleCommand* command) {
    if (command != NULL) {
        free( command->domain );
        free( command->operational_label );
        free( command->original );
        free( command->answer );
        free( command->json );
        free( command );
    }
}

char** split_and_metaphone(const char* input, int* count) {
    int word_count = 0;
    const char* temp = input;

    while (*temp) {
        while (*temp == ' ' && *temp) temp++;
        if (*temp) word_count++;
        while (*temp != ' ' && *temp) temp++;
    }

    char** result = malloc(word_count * sizeof(char*));
    const char* start = input;
    const char* end = input;
    int index = 0;

    while (*end) {
        while (*end == ' ' && *end) end++;
        start = end;
        while (*end != ' ' && *end) end++;
        if (start != end) {
            size_t length = end - start;
            char* word = malloc(length + 1);
            strncpy(word, start, length);
            word[length] = '\0';

            char *resulta = NULL;
            metaphone(word, strlen(word), 0, &resulta, 1);

            result[index++] = resulta;
            free(word);
        }
    }

    *count = word_count;
    return result;
}

char** split_to_words(const char* input, int* count) {
    int word_count = 0;
    const char* temp = input;

    while (*temp) {
        while (*temp == ' ' && *temp) temp++;
        if (*temp) word_count++;
        while (*temp != ' ' && *temp) temp++;
    }

    char** result = malloc(word_count * sizeof(char*));
    const char* start = input;
    const char* end = input;
    int index = 0;

    while (*end) {
        while (*end == ' ' && *end) end++;
        start = end;
        while (*end != ' ' && *end) end++;
        if (start != end) {
            size_t length = end - start;
            result[index] = malloc(length + 1);
            strncpy(result[index], start, length);
            result[index][length] = '\0';
            index++;
        }
    }

    *count = word_count;
    return result;
}

void clean_string(const char *input, char *output) {
    int j = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        if (isalnum((unsigned char)input[i]) || input[i] == ' ') {
            output[j++] = tolower((unsigned char)input[i]);
        }
    }
    output[j] = '\0';
}

int compare(int distance_a, int distance_b) {
    if (distance_b < distance_a) {
        return -1;
    } else if (distance_b > distance_a) {
        return 1;
    } else {
        return 0;
    }
}

void simplify_string(char* str) {
    char *dst = str, *src = str;
    while (*src) {
        if (strchr("`ʻʼ'‘’‛′ʽ,\"", *src) == NULL) {
            *dst = tolower(*src);
            dst++;
        }
        src++;
    }
    *dst = '\0';
}

void init_smilarity_cache(int size) {
    similarity_cache_size = size;
    similarity_cache = (SimilarityCache *)malloc( similarity_cache_size * sizeof( SimilarityCache ) );
}

void free_smilarity_cache() {
    free( similarity_cache );
}

int compare_words(const void *a, const void *b) {
    const char *word1 = *(const char **)a;
    const char *word2 = *(const char **)b;
    int len1 = strlen(word1);
    int len2 = strlen(word2);

    int scoreA = similar_char(word1, len1, global_keyword, strlen(global_keyword));
    int scoreB = similar_char(word2, len2, global_keyword, strlen(global_keyword));

    double scoreAp = scoreA * 200.0 / (strlen(global_keyword) + len1);
    double scoreBp = scoreB * 200.0 / (strlen(global_keyword) + len2);

    if (scoreAp > scoreBp) return -1;
    if (scoreAp < scoreBp) return 1;

    return strcmp(word1, word2);
}

char *tokenize_and_sort(const char *txt) {
    char *copy = strdup(txt);
    if (!copy) return NULL;

    char *token = strtok(copy, " ");
    char **words = NULL;
    int count = 0;

    while (token) {
        words = (char **)realloc(words, (count + 1) * sizeof(char *));
        words[count++] = strdup(token);
        token = strtok(NULL, " ");
    }

    qsort(words, count, sizeof(char *), compare_words);

    size_t total_length = 0;
    for (int i = 0; i < count; i++) {
        total_length += strlen(words[i]) + 1;
    }

    char *sorted_text = (char *)malloc(total_length);
    if (!sorted_text) {
        free(copy);
        for (int i = 0; i < count; i++) {
            free(words[i]);
        }
        free(words);
        return NULL;
    }

    sorted_text[0] = '\0';
    for (int i = 0; i < count; i++) {
        strcat(sorted_text, words[i]);
        if (i < count - 1) {
            strcat(sorted_text, " ");
        }
    }

    free(copy);
    for (int i = 0; i < count; i++) {
        free(words[i]);
    }
    free(words);

    return sorted_text;
}

int get_similarity(const char *txt1, int len1, const char *txt2, int len2) {
    unsigned int hash = 5381;

    for (int i = 0; i < len1; i++) {
        hash = ((hash << 5) + hash) + txt1[i];
    }
    for (int i = 0; i < len2; i++) {
        hash = ((hash << 5) + hash) + txt2[i];
    }
    int index = hash % similarity_cache_size;

    if (similarity_cache[index].hash == hash) {
        return similarity_cache[index].similarity;
    }

    char *sorted_txt1 = tokenize_and_sort(txt1);
    char *sorted_txt2 = tokenize_and_sort(txt2);

    if (!sorted_txt1 || !sorted_txt2) {
        if (sorted_txt1) free(sorted_txt1);
        if (sorted_txt2) free(sorted_txt2);
        return 0;
    }

    int max = similar_char(sorted_txt1, strlen(sorted_txt1), sorted_txt2, strlen(sorted_txt2));

    similarity_cache[index].hash = hash;
    similarity_cache[index].similarity = max;

    free(sorted_txt1);
    free(sorted_txt2);

    return max;
}

int compare_commands(const void *a, const void *b) {
    const Command *cmd_a = (const Command *)a;
    const Command *cmd_b = (const Command *)b;

    char temp_a[256], temp_b[256];
    strcpy(temp_a, cmd_a->text);
    strcpy(temp_b, cmd_b->text);

    simplify_string(temp_a);
    simplify_string(temp_b);

    int keyword_len = strlen(global_keyword);
    int a_len = strlen(temp_a);
    int b_len = strlen(temp_b);

    int scoreA = get_similarity(global_keyword, keyword_len, temp_a, a_len);
    int scoreB = get_similarity(global_keyword, keyword_len, temp_b, b_len);

    double scoreAp = scoreA * 200.0 / (keyword_len + a_len);
    double scoreBp = scoreB * 200.0 / (keyword_len + b_len);

    if (scoreAp > scoreBp) return -1;
    if (scoreAp < scoreBp) return 1;

    return 0;
}

/*int get_similarity(const char *txt1, int len1, const char *txt2, int len2) {
    unsigned int hash = 5381;

    for ( int i = 0; i < len1; i++ ) {
        hash = ( ( hash << 5 ) + hash ) + txt1[i];
    }
    for ( int i = 0; i < len2; i++ ) {
        hash = ( ( hash << 5 ) + hash ) + txt2[i];
    }
    int index = hash % similarity_cache_size;

    if ( similarity_cache[index].hash == hash ) {
        return similarity_cache[index].similarity;
    }
    
    int max = similar_char(txt1, len1, txt2, len2);


    similarity_cache[index].hash = hash;
    similarity_cache[index].similarity = max;
    
    return max;
}

int compare_commands(const void *a, const void *b) {
    const Command *cmd_a = (const Command *)a;
    const Command *cmd_b = (const Command *)b;

    simplify_string( (char *)cmd_a->text );
    simplify_string( (char *)cmd_b->text );

    int keyword_len = strlen( global_keyword );
    int a_len = strlen( cmd_a->text );
    int b_len = strlen( cmd_b->text );

    int scoreA = get_similarity( global_keyword, keyword_len, cmd_a->text, a_len );
    int scoreB = get_similarity( global_keyword, keyword_len, cmd_b->text, b_len );

    double scoreAp = scoreA * 200.0 / (keyword_len + a_len);
    double scoreBp = scoreB * 200.0 / (keyword_len + b_len);

    if (scoreAp > scoreBp) return -1;
    if (scoreAp < scoreBp) return 1;

    return 0;
}*/

/*int compare_commands(const void *a, const void *b) {
    const Command *cmd_a = (const Command *)a;
    const Command *cmd_b = (const Command *)b;

    simplify_string( (char *)cmd_a->text );
    simplify_string( (char *)cmd_b->text );

    int keyword_len = strlen(global_keyword);
    int a_len = strlen(cmd_a->text);
    int b_len = strlen(cmd_b->text);

    int scoreA = similar_char(global_keyword, keyword_len, cmd_a->text, a_len);
    int scoreB = similar_char(global_keyword, keyword_len, cmd_b->text, b_len);

    double scoreAp = scoreA * 200.0 / (keyword_len + a_len);
    double scoreBp = scoreB * 200.0 / (keyword_len + b_len);

    if (scoreAp > scoreBp) return -1;
    if (scoreAp < scoreBp) return 1;

    return 0;
}*/

//sorting

int ucompare(const void *a, const void *b, char *keyword) {
    const Command *dataA = (const Command *)a;
    const Command *dataB = (const Command *)b;

    char aText[100];
    char bText[100];
    strcpy(aText, dataA->text);
    strcpy(bText, dataB->text);

    char *ptr;
    const char *chars = "ʻʼ'‘’‛′ʽ`,\"";
    for (ptr = aText; *ptr; ++ptr)
        if (strchr(chars, *ptr)) *ptr = ' ';
    for (ptr = bText; *ptr; ++ptr)
        if (strchr(chars, *ptr)) *ptr = ' ';

    for (ptr = aText; *ptr; ++ptr) *ptr = tolower(*ptr);
    for (ptr = bText; *ptr; ++ptr) *ptr = tolower(*ptr);

    char *posA = strstr(aText, keyword);
    char *posB = strstr(bText, keyword);

    if (posA != NULL && posB == NULL) return -1;
    if (posB != NULL && posA == NULL) return 1;
    if (posA != NULL && posB != NULL) return (posA - aText) - (posB - bText);

    int scoreA = similar_char(keyword, strlen(keyword), aText, strlen(aText));
    int scoreB = similar_char(keyword, strlen(keyword), bText, strlen(bText));

    if (scoreA > scoreB) return -1;
    if (scoreA < scoreB) return 1;
    return 0;
}

void swap(Command *a, Command *b) {
    Command temp = *a;
    *a = *b;
    *b = temp;
}

int partition(Command *arr, int low, int high, char *keyword) {
    Command pivot = arr[high];
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        if (ucompare(&arr[j], &pivot, keyword) <= 0) {
            i++;
            swap(&arr[i], &arr[j]);
        }
    }
    swap(&arr[i + 1], &arr[high]);
    return (i + 1);
}

void quicksort(Command *arr, int low, int high, char *keyword) {
    if (low < high) {
        int pi = partition(arr, low, high, keyword);
        quicksort(arr, low, pi - 1, keyword);
        quicksort(arr, pi + 1, high, keyword);
    }
}

Command *sort_database(Command *database, int n, char *keyword) {
    quicksort(database, 0, n - 1, keyword);
    return database;
}

// end sorting

int split(const char* str, char words[][MAX_LEN]) {
    int n = 0;
    const char* separator = " ";
    char* token;
    char strCopy[1024];

    strcpy(strCopy, str);
    token = strtok(strCopy, separator);

    while (token != NULL) {
        strcpy(words[n++], token);
        token = strtok(NULL, separator);
    }

    return n;
}

int min(int a, int b, int c) {
    if (a < b && a < c) return a;
    if (b < a && b < c) return b;
    return c;
}

int levenshtein_distance(char *s1, char *s2) {
    int len1 = strlen(s1), len2 = strlen(s2);
    int matrix[len1 + 1][len2 + 1];
    
    for (int i = 0; i <= len1; i++) matrix[i][0] = i;
    for (int j = 0; j <= len2; j++) matrix[0][j] = j;
    
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (s1[i - 1] == s2[j - 1]) ? 0 : 1;
            matrix[i][j] = min(
                matrix[i - 1][j] + 1,
                matrix[i][j - 1] + 1,
                matrix[i - 1][j - 1] + cost
            );
        }
    }
    
    return matrix[len1][len2];
}

void set_nested_value(cJSON *json, const char *path, const char *value) {
    char *mutablePath = strdup(path);
    char *token = strtok(mutablePath, ".");
    cJSON *current = json;
    cJSON *parent = NULL;
    char *previousToken = NULL;

    while (token != NULL) {
        parent = current;
        current = cJSON_GetObjectItemCaseSensitive(current, token);

        if (current == NULL) {
            current = cJSON_CreateObject();
            cJSON_AddItemToObject(parent, token, current);
        }

        previousToken = token;
        token = strtok(NULL, ".");
        if (token == NULL) {
            cJSON *item = cJSON_GetObjectItemCaseSensitive(parent, previousToken);
            if (item != NULL) {
                cJSON_DeleteItemFromObject(parent, previousToken);
                cJSON_AddStringToObject(parent, previousToken, value);
            } else {
                cJSON_AddStringToObject(parent, previousToken, value);
            }
        }
    }

    free(mutablePath);
}

char* remove_duplicate_words(char* str) {
    char* result = (char*)malloc(strlen(str) + 1);
    if (result == NULL) {
        printf("Memory allocation failed.\n");
        return NULL;
    }
    result[0] = '\0';

    const char* delimiters = " ";
    char* token = strtok(str, delimiters);
    while (token != NULL) {
        if (!strstr(result, token)) {
            strcat(result, token);
            strcat(result, " ");
        }
        token = strtok(NULL, delimiters);
    }

    size_t len = strlen(result);
    if (len > 0) {
        result[len - 1] = '\0';
    }

    return result;
}

char* str_to_lower(char* s) {
  for(char *p=s; *p; p++) *p=tolower(*p);
  return s;
}

char* str_to_upper(char* s) {
  for(char *p=s; *p; p++) *p=toupper(*p);
  return s;
}

int find_matches(const char* str, const char* pattern, RegexMatches** outMatches) {
    int captureCount = 0;
    const int maxCaptures = 128;
    const int maxGroups = 10;

    *outMatches = (RegexMatches*)malloc(maxCaptures * sizeof(RegexMatches));
    if (*outMatches == NULL) return -1;

    struct re_cap caps[maxGroups];
    int i, j = 0, str_len = strlen(str);
    printf("%s\n", pattern);
    while (j < str_len &&
           (i = re_match(pattern, str + j, str_len - j, caps, maxGroups, RE_IGNORE_CASE)) > 0) {
        for (int groupCount = 0; groupCount < maxGroups && caps[groupCount].len > 0; ++groupCount) {
            size_t length = caps[groupCount].len;
            (*outMatches)[captureCount].result = (char*)malloc(length + 1);
            if ((*outMatches)[captureCount].result == NULL) break;
            strncpy((*outMatches)[captureCount].result, caps[groupCount].ptr, length);
            (*outMatches)[captureCount].result[length] = '\0';
            captureCount++;

            if (captureCount >= maxCaptures) break;
        }
        j += i;
        if (captureCount >= maxCaptures) break;
    }

    return captureCount;
}

char* str_replace(const char* subject, const char* search, const char* replace) {
    size_t subject_len = strlen(subject);
    size_t search_len = strlen(search);
    size_t replace_len = strlen(replace);

    size_t count = 0;
    const char* tmp = subject;
    while ((tmp = strstr(tmp, search)) != NULL) {
        count++;
        tmp += search_len;
    }

    size_t result_len = subject_len + count * (replace_len - search_len) + 1;
    char* result = (char*)malloc(result_len);
    if (!result) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    char* dest = result;
    const char* src = subject;

    while (*src) {
        if (strstr(src, search) == src) {
            strcpy(dest, replace);
            dest += replace_len;
            src += search_len;
        } else {
            *dest++ = *src++;
        }
    }

    *dest = '\0';

    return result;
}

char* remove_substring(char *str, const char *toRemove) {
    int len = strlen(toRemove);
    char *found;

    while ((found = strstr(str, toRemove)) != NULL) {
        memmove(found, found + len, strlen(found + len) + 1);
    }

    return str;
}

char *concat(char *a, char *b) {
    int lena = strlen(a);
    int lenb = strlen(b);
    char *con = malloc(lena+lenb+1);
    memcpy(con,a,lena);
    memcpy(con+lena,b,lenb+1);        
    return con;
}

char* trim( char* str, const char ch, int mode ) {
    if (str == NULL)
        return NULL;
    
    char tmp[ 1024 * 10 ] = { 0x00 };
    
    strcpy(tmp, str);
    
    int len = strlen(tmp);
    
    char* start = tmp;
    char* end = tmp+len;
    
    for( int i = 0; i < len; i++ ){
        if ( tmp[i] == ch && start == tmp+i && mode != 2 )
            ++start;

        if ( tmp[len-i-1] == ch && end == tmp+len-i && mode != 1 )
            *(--end) = '\0';
    }

    return start;
}

int is_directory_exists(const char *path) {
    struct stat stats;
    stat(path, &stats);
    
    if (S_ISDIR(stats.st_mode))
        return 1;

    return 0;
}

void replace_white_space(char *str) {
    char *ptr = str;
    while (*ptr) {
        if (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') {
            *ptr = ' ';
        }
        ptr++;
    }
}

double match(char *s1, char *s2, bool comparePhonetics) {
    replace_white_space(s1);
    replace_white_space(s2);

    char *words1[100];
    char *words2[100];
    int words1_count = 0, words2_count = 0;

    char *token = strtok(s1, " ");
    
    while (token != NULL && words1_count < 100) {
        words1[words1_count++] = token;
        token = strtok(NULL, " ");
    }

    token = strtok(s2, " ");
    while (token != NULL && words2_count < 100) {
        words2[words2_count++] = token;
        token = strtok(NULL, " ");
    }

    if (comparePhonetics) {
        for (int i = 0; i < words1_count; i++) {
            words1[i] = umetaphone(words1[i]);
        }
        for (int i = 0; i < words2_count; i++) {
            words2[i] = umetaphone(words2[i]);
        }
    }

    double totalWeight = 0;
    for (int i = 0; i < words1_count; i++) {
        double maxWeight = -1;
        int foundIndex = 0;
        for (int j = 0; j < words2_count; j++) {
            if (strcmp(words2[j], "") == 0) continue;
            double weight = 100 * jaro_winkler_distance(words1[i], words2[j], 0.25);
            if (weight > maxWeight) {
                maxWeight = weight;
                foundIndex = j;
            }
        }
        if (maxWeight > 70) {
            totalWeight += maxWeight;
            words2[foundIndex] = "";
        }
    }

    return totalWeight / (words1_count * 100);
}