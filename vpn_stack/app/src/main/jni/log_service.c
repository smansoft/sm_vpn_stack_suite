/*
 *    Copyright (c) 2021 SManSoft <http://www.smansoft.com/>
 *    Sergey Manoylo <info@smansoft.com>
 */

#include <jni.h>

#include "sm_files_tools.h"
#include "sm_log.h"

sm_log_config gsm_log_config;           //  global instance of main log support structure
#define SM_LOG_CONFIG &gsm_log_config   //  just synonym: SM_LOG_CONFIG == &gsm_log_config - for usage in log api calls

/**
 *
 * @param env
 * @param clazz
 * @param _log_dir_path
 * @param _log_file_name
 * @return
 */
JNIEXPORT jint JNICALL
Java_com_smansoft_vpn_1stack_LogService_logInit(JNIEnv *env, jclass clazz, jstring _log_dir_path,
                                                jstring _log_file_name) {

    jint result = -1;

    size_t log_dir_path_length = (*env)->GetStringLength(env, _log_dir_path);
    if(log_dir_path_length <= 0) {
        return result;
    }

    size_t log_file_name_length = (*env)->GetStringLength(env, _log_file_name);
    if(log_file_name_length <= 0) {
        return result;
    }

    const char *log_dir_path = (*env)->GetStringUTFChars(env, _log_dir_path, NULL);
    const char *log_file_name = (*env)->GetStringUTFChars(env, _log_file_name, NULL);

    if(!log_dir_path || !log_file_name)
        return result;

    result = sm_log_init_dpath_fname(SM_LOG_CONFIG,
                                     SM_LOG_OUT_FILE,
                                     SM_LOG_LEVEL_OFF,
                                     SM_LOG_LEVEL_ALL,
                                     log_dir_path,
                                     log_file_name);
#if 0
    err = sm_log_init_dpath_fname(SM_LOG_CONFIG,
                                  SM_LOG_OUT_FILE,
                                  SM_LOG_LEVEL_OFF,
                                  SM_LOG_LEVEL_INFO,
                                  log_dir_path,
                                  log_file_name);
#endif

    (*env)->ReleaseStringUTFChars(env, _log_dir_path, log_dir_path);
    (*env)->ReleaseStringUTFChars(env, _log_file_name, log_file_name);

    if (result == SM_RES_OK)
        result = sm_log_start(SM_LOG_CONFIG);  // starting created log
    return result;
}

/**
 *
 * @param env
 * @param clazz
 * @return
 */
JNIEXPORT jint JNICALL
Java_com_smansoft_vpn_1stack_LogService_logClose(JNIEnv *env, jclass clazz) {
    sm_log_stop(SM_LOG_CONFIG);
    return sm_log_close(SM_LOG_CONFIG);
}

/**
 *
 * @param env
 * @param clazz
 * @param _log_level
 * @param _log_category
 * @param _log_text
 * @return
 */
JNIEXPORT jint JNICALL
Java_com_smansoft_vpn_1stack_LogService_logPrint(JNIEnv *env, jclass clazz, jint _log_level,
                                                 jstring _log_category, jstring _log_text) {

    jint result = -1;

    size_t log_category_length = (*env)->GetStringLength(env, _log_category);
    if(log_category_length <= 0) {
        return result;
    }

    size_t log_text_length = (*env)->GetStringLength(env, _log_text);
    if(log_text_length <= 0) {
        return result;
    }

    const char *log_category = (*env)->GetStringUTFChars(env, _log_category, NULL);
    const char *log_text = (*env)->GetStringUTFChars(env, _log_text, NULL);

    if(!log_category || !log_text)
        return result;

    result =  sm_log_print(SM_LOG_CONFIG,
                           _log_level,
                           log_category,
                           log_text);

    (*env)->ReleaseStringUTFChars(env, _log_category, log_category);
    (*env)->ReleaseStringUTFChars(env, _log_text, log_text);

    return result;
}
