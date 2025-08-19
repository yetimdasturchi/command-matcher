#ifndef MAIN_H_
#define MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#ifdef FOR_DROID
#include <jni.h>
#endif
#include "utils.h"
#include "global.h"
#include "macros.h"
#include "jaro.h"

Command* commands = NULL;
SingleCommand* command = NULL;

int commands_size;
char global_keyword[100];
char * res_path;

SimilarityCache* similarity_cache;
int similarity_cache_size = 0;

#define SUCCESS 0
#define NO_INPUT -1
#define FAILED_OPEN_DATABASE -2
#define FAILED_LOAD_DATABASE -3
#define COMMAND_NOT_DEFINED -4
#define ERROR_RES -5

__attribute__((visibility("default"))) char * get_domain();
__attribute__((visibility("default"))) char * get_operational_label();
__attribute__((visibility("default"))) char * get_original();
__attribute__((visibility("default"))) char * get_answer();
__attribute__((visibility("default"))) char * get_json();
__attribute__((visibility("default"))) int init_res( char * dir );
__attribute__((visibility("default"))) int get_from_text( char * input );

#ifdef FOR_DROID
__attribute__((visibility("default"))) JNIEXPORT jstring JNICALL Java_uz_manu_command_matcher_getDomain(JNIEnv *env, jobject obj);
__attribute__((visibility("default"))) JNIEXPORT jstring JNICALL Java_uz_manu_command_matcher_getOperationalLabel(JNIEnv *env, jobject obj);
__attribute__((visibility("default"))) JNIEXPORT jstring JNICALL Java_uz_manu_command_matcher_getIndex(JNIEnv *env, jobject obj);
__attribute__((visibility("default"))) JNIEXPORT jstring JNICALL Java_uz_manu_command_matcher_getAnswer(JNIEnv *env, jobject obj);
__attribute__((visibility("default"))) JNIEXPORT jstring JNICALL Java_uz_manu_command_matcher_getJson(JNIEnv *env, jobject obj);
__attribute__((visibility("default"))) JNIEXPORT jint JNICALL Java_uz_manu_command_matcher_initRes(JNIEnv *env, jobject obj, jstring dir);
__attribute__((visibility("default"))) JNIEXPORT jint JNICALL Java_uz_manu_command_matcher_getFromText(JNIEnv *env, jobject obj, jstring input);
__attribute__((visibility("default"))) JNIEXPORT void JNICALL Java_uz_manu_command_matcher_unInitRes(JNIEnv *envj, jobject obj);
#endif

#endif // MAIN_H_