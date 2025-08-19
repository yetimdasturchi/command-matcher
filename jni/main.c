#include "main.h"
#include <time.h>

int init_res(char * dir) {

    char * trimmed_dir = malloc( strlen( dir ) * sizeof( char ) + 1 );
  
    strcpy( trimmed_dir, concat( trim( dir, '/', 2 ), "/" ) );

    int len  = strlen( trimmed_dir ) * sizeof( char );
    res_path = realloc( res_path, len );

    /*if ( is_utf8( trimmed_dir ) ) {
        strcpy( res_path, trimmed_dir );
    }else{
        gbk_to_utf8( trimmed_dir, res_path, len );
    }*/

    strcpy( res_path, trimmed_dir );

    if ( ! is_directory_exists( res_path ) )
        return ERROR_RES;

    if ( init_database() != 0 )
        return FAILED_OPEN_DATABASE;

    if ( load_commands() != 0 )
        return FAILED_LOAD_DATABASE;

    return SUCCESS;
}

char * get_domain() {
    return command->domain;
}

char * get_operational_label(){
    return command->operational_label;
}

char * get_original(){
    return command->original;
}

char * get_answer(){
    return command->answer;
}

char * get_json(){
    return command->json;
}

int get_from_text( char * input ) {
    if ( input == NULL || strlen( input ) == 0 ) 
        return NO_INPUT;

    clean_string( input, global_keyword );
    init_smilarity_cache( commands_size );
    qsort( commands, commands_size, sizeof( Command ), compare_commands );

    char clean_command[100];
    clean_string( commands[0].text, clean_command );
    
    if ( match( global_keyword, clean_command, false ) < 0.75  )
        return COMMAND_NOT_DEFINED;

    command = get_single_command( commands[0].id );

    if (command == NULL) 
        return COMMAND_NOT_DEFINED;

    const char *jsonString = command->json;
    cJSON *json = cJSON_Parse( jsonString );

    if ( strlen( command->regex ) > 0 ) {
        RegexMatches* re_matches;
        int re_matches_count = find_matches(input, command->regex, &re_matches);

        if ( re_matches_count == -1 )
            return COMMAND_NOT_DEFINED;

        MacrosMatches* ma_matches = NULL;
        int ma_count = 0;

        fetch_macros_matches(&ma_matches, &ma_count, commands[0].id);

        if ( ma_count == 0 ) 
            return COMMAND_NOT_DEFINED;

        for (int i = 0; i < ma_count; i++) {
            set_nested_value(
                json, 
                ma_matches[i].key,
                to_macros(
                    re_matches[ ma_matches[i].value ].result,
                    ma_matches[i].macros
                )
            );

            free(ma_matches[i].key);
            free(ma_matches[i].macros);
        }

        for (int i = 0; i < re_matches_count; ++i) {
            free(re_matches[i].result);
        }

        free(re_matches);
        free(ma_matches);
    }

    char *modified_json_string = cJSON_Print(json);
    command->json = strdup(cJSON_Print(json));
        
    cJSON_Delete(json);
    free(modified_json_string);
    free_smilarity_cache();
    return SUCCESS;
}

void un_init_res() {
    free_single_command( command );
    close_database();
}

#ifdef FOR_DROID
JNIEXPORT jstring JNICALL Java_uz_manu_command_matcher_getDomain(JNIEnv *env, jobject obj) {
    return (*env)->NewStringUTF(env, get_domain());
}

JNIEXPORT jstring JNICALL Java_uz_manu_command_matcher_getOperationalLabel(JNIEnv *env, jobject obj) {
    return (*env)->NewStringUTF(env, get_operational_label());
}

JNIEXPORT jstring JNICALL Java_uz_manu_command_matcher_getIndex(JNIEnv *env, jobject obj) {
    return (*env)->NewStringUTF(env, get_original());
}

JNIEXPORT jstring JNICALL Java_uz_manu_command_matcher_getAnswer(JNIEnv *env, jobject obj) {
    return (*env)->NewStringUTF(env, get_answer());
}

JNIEXPORT jstring JNICALL Java_uz_manu_command_matcher_getJson(JNIEnv *env, jobject obj) {
    return (*env)->NewStringUTF(env, get_json());
}

JNIEXPORT jint JNICALL Java_uz_manu_command_matcher_initRes(JNIEnv *env, jobject obj, jstring dir) {
    const char *native_dir = (*env)->GetStringUTFChars(env, dir, 0);
    int result = init_res((char*)native_dir);
    (*env)->ReleaseStringUTFChars(env, dir, native_dir);
    return result;
}

JNIEXPORT jint JNICALL Java_uz_manu_command_matcher_getFromText(JNIEnv *env, jobject obj, jstring input) {
    const char *native_input = (*env)->GetStringUTFChars(env, input, 0);
    int result = get_from_text((char*)native_input);
    (*env)->ReleaseStringUTFChars(env, input, native_input);
    return result;
}

JNIEXPORT void JNICALL Java_uz_manu_command_matcher_unInitRes(JNIEnv *envj, jobject obj) {
    un_init_res();
}
#endif