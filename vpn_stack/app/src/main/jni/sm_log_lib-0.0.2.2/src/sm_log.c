/*
 *    Copyright (c) 2020-2021 SManSoft <http://www.smansoft.com/>
 *    Sergey Manoylo <info@smansoft.com>
 */

#include "pch.h"

#include <pthread.h>

#include "safe_str_lib.h"
#include "safe_mem_lib.h"
#include "safe_fopen.h"

#include "sm_log_types.h"
#include "sm_files_tools.h"
#include "sm_log.h"

#if !defined SM_LOG_LIB_FORMAT
 /*
     format of log output:
         date :          %02d/%02d/%04d
         time :          %02d:%02d:%02d.%03d
         category:       <%s>
         log message:    %s
 */
#define SM_LOG_LIB_FORMAT              "%02d/%02d/%04d %02d:%02d:%02d.%03d - %s - <%s> - %s\n"
#endif

#if !defined SM_LOG_SBUFF_SIZE
#define SM_LOG_SBUFF_SIZE               (32)            //  small buffer
#endif

#if !defined SM_LOG_BUFF_SIZE
#define SM_LOG_BUFF_SIZE               (10*1024)        //  max length of log message
#endif

int gsm_count_lock = 0;

#if defined SM_SYNC_LOG

#if defined SM_OS_LINUX

 /*
     Implementation of synchronizing functions of log (Linux suite);
     Implamentation based on usage of functionality, supported by C11 (ISO/IEC 9899:2011);
 */

 /*  initializing the multi-platform synchronizing object    */
errno_t sm_sync_type_init(sm_sync_type* const sync_type)
{
    errno_t err = SM_RES_OK;
    pthread_mutexattr_t mtx_attr;
        int mtx_res = pthread_mutexattr_init(&mtx_attr);
        if (mtx_res)
            return SM_RES_ERROR;
        mtx_res = pthread_mutexattr_settype(&mtx_attr, PTHREAD_MUTEX_RECURSIVE_NP);
//  mtx_res = pthread_mutexattr_settype(&mtx_attr, PTHREAD_MUTEX_DEFAULT);
//  mtx_res = pthread_mutexattr_settype(&mtx_attr, PTHREAD_MUTEX_NORMAL);
        if (mtx_res)
            return SM_RES_ERROR;
    mtx_res = pthread_mutex_init(&sync_type->m_sync, &mtx_attr);
//  int mtx_res = pthread_mutex_init(&sync_type->m_sync, NULL);
    err = (!mtx_res) ? SM_RES_OK : SM_RES_ERROR;
    return err;
}

/*  destroying the multi-platform synchronizing object  */
errno_t sm_sync_type_close(sm_sync_type* const sync_type)
{
    errno_t err = SM_RES_OK;
    int mtx_res = pthread_mutex_destroy(&sync_type->m_sync);
    err = (!mtx_res) ? SM_RES_OK : SM_RES_ERROR;
    return err;
}

/*  locking the current thread, using multi-platform synchronizing object   */
errno_t sm_sync_type_lock(sm_sync_type* const sync_type)
{
    errno_t err = SM_RES_OK;
    int mtx_res = pthread_mutex_lock(&sync_type->m_sync);
    err = (!mtx_res) ? SM_RES_OK : SM_RES_ERROR;
    return err;
}

/*
    checking for lock the current thread, using multi-platform synchronizing object;
    result:
        SM_RES_OK       - lock is poissible
        SM_RES_ERROR    - lock isn't poissible
*/
errno_t sm_sync_type_try_lock(sm_sync_type* const sync_type)
{
    errno_t err = SM_RES_OK;
//  int mtx_res = pthread_mutex_trylock(&sync_type->m_sync);
//  err = (!mtx_res) ? SM_RES_OK : SM_RES_ERROR;
    return err;
}

/*  unlocking the current thread, using multi-platform synchronizing object   */
errno_t sm_sync_type_unlock(sm_sync_type* const sync_type)
{
    errno_t err = SM_RES_OK;
    int mtx_res = pthread_mutex_unlock(&sync_type->m_sync);
    err = (!mtx_res) ? SM_RES_OK : SM_RES_ERROR;
    return err;
}

#elif defined SM_OS_WINDOWS

 /*
     Implementation of synchronizing functions of log (Windows suite)
 */

 /*  initializing the multi-platform synchronizing object    */
errno_t sm_sync_type_init(sm_sync_type* const sync_type)
{
    errno_t err;
    gsm_count_lock = 0;
    __try {
        InitializeCriticalSection(&sync_type->m_sync);
        err = SM_RES_OK;
    }
    __except (GetExceptionCode()) {
        err = SM_RES_ERROR;
    }
    return err;
}

/*  destroying the multi-platform synchronizing object  */
errno_t sm_sync_type_close(sm_sync_type* const sync_type)
{
    errno_t err;
    __try {
        DeleteCriticalSection(&sync_type->m_sync);
        err = SM_RES_OK;
    }
    __except (GetExceptionCode()) {
        err = SM_RES_ERROR;
    }
    return err;
}

/*  locking the current thread, using multi-platform synchronizing object   */
errno_t sm_sync_type_lock(sm_sync_type* const sync_type)
{
    errno_t err;
    __try {
        gsm_count_lock++;
        EnterCriticalSection(&sync_type->m_sync);
        err = SM_RES_OK;
    }
    __except (GetExceptionCode()) {
        err = SM_RES_ERROR;
    }
    return err;
}

/*
    checking for lock the current thread, using multi-platform synchronizing object;
    result:
        SM_RES_OK       - lock is poissible
        SM_RES_ERROR    - lock isn't poissible
*/
errno_t sm_sync_type_try_lock(sm_sync_type* const sync_type)
{
    errno_t err;
    __try {
        err = (TryEnterCriticalSection(&sync_type->m_sync)) ? SM_RES_OK : SM_RES_ERROR;
    }
    __except (GetExceptionCode()) {
        err = SM_RES_ERROR;
    }
    return err;
}

/*  unlocking the current thread, using multi-platform synchronizing object   */
errno_t sm_sync_type_unlock(sm_sync_type* const sync_type)
{
    errno_t err;
    __try {
        gsm_count_lock--;
        LeaveCriticalSection(&sync_type->m_sync);
        err = SM_RES_OK;
    }
    __except (GetExceptionCode()) {
        err = SM_RES_ERROR;
    }
    return err;
}

#endif

#endif

/*
    initializing the sm_log_config object, using one argument: log file path;
    also initalizing sm_sync_type object (m_sync), if SM_SYNC_LOG is defined;

    args:
        log_config      - sm_log_config object
        log_out         - log output (file and/or console)
        min_log_level   - min log level
        max_log_level   - max log level
        log_fpath       - path of the log file
    result:
        SM_RES_OK
        SM_RES_ERROR
*/
errno_t sm_log_init_fpath(sm_log_config* const log_config,
        const unsigned int log_out,
        const unsigned int min_log_level, const unsigned int max_log_level,
        const char* const log_fpath)
{
    errno_t err = SM_RES_ERROR;

    if (!log_config)
        return err;

    if(min_log_level > max_log_level)
        return err;

    safe_memset(log_config, sizeof(sm_log_config), 0);

#if defined SM_SYNC_LOG
        err = sm_sync_type_init(&log_config->m_sync);
        if (err == SM_RES_ERROR)
            return err;
        sm_sync_type_lock(&log_config->m_sync);
#endif

    while(true) {
        err = SM_RES_OK;
        if(log_out & SM_LOG_OUT_FILE) {
            size_t log_fpath_len = safe_strnlen(log_fpath, MAX_PATH);
            if (log_fpath_len > 0) {
                char log_fpath_abs[MAX_PATH] = { 0 };
                err = sm_make_path_abs(log_fpath_abs, SM_ARRAY_SIZE(log_fpath_abs), log_fpath, SM_BIN_PATH_MARKER);
                if (err != SM_RES_OK)
                    break;
                size_t log_fpath_abs_len = safe_strnlen(log_fpath_abs, MAX_PATH);
                if (log_fpath_abs_len <= 0) {
                    err = SM_RES_ERROR;
                    break;
                }
                //  init of log_config->m_log_fpath
                errno_t err_safe = safe_strncpy(log_config->m_log_fpath, MAX_PATH, log_fpath_abs, log_fpath_abs_len);
                if (err_safe) {
                    err = SM_RES_ERROR;
                    break;
                }
                char log_dpath[MAX_PATH] = { 0 };
                char log_fname[MAX_PATH] = { 0 };
                // (*)
                err = sm_get_dpath_from_fpath(log_dpath, SM_ARRAY_SIZE(log_dpath), log_fpath_abs);
                if (err != SM_RES_OK)
                    break;
                err = sm_get_fname_from_fpath(log_fname, SM_ARRAY_SIZE(log_fname), log_fpath_abs);
                if (err != SM_RES_OK)
                    break;
                size_t log_fname_len = safe_strnlen(log_fname, MAX_PATH);
                if (log_fname_len <= 0) {
                    err = SM_RES_ERROR;
                    break;
                }
                err = sm_log_init_dpath_fname(log_config, log_out, min_log_level, max_log_level, log_dpath, log_fname);
            }
        }
        log_config->m_log_out = log_out;
        log_config->m_min_log_level = min_log_level;
        log_config->m_max_log_level = max_log_level;
        break;
    }
#if defined SM_SYNC_LOG
    sm_sync_type_unlock(&log_config->m_sync);
#endif
    return err;
}

/*
    initializing the sm_log_config object;
    also initalizing sm_sync_type object (m_sync), if SM_SYNC_LOG is defined;

    args:
        log_config      - sm_log_config object
        log_out         - log output (file and/or console)
        min_log_level   - min log level
        max_log_level   - max log level
        log_dpath       - directory, where log file will be created
        log_fname       - file name of log file
    result:
        SM_RES_OK
        SM_RES_ERROR
*/
errno_t sm_log_init_dpath_fname(sm_log_config* const log_config,
        const unsigned int log_out,
        const unsigned int min_log_level, const unsigned int max_log_level,
        const char* const log_dpath, const char* const log_fname)
{
    errno_t err = SM_RES_ERROR;

    if (!log_config)
        return err;

    if(min_log_level > max_log_level)
        return err;

    safe_memset(log_config, sizeof(sm_log_config), 0);

#if defined SM_SYNC_LOG
    err = sm_sync_type_init(&log_config->m_sync);
    if (err == SM_RES_ERROR)
        return err;
    sm_sync_type_lock(&log_config->m_sync);
#endif

    while(true) {
        err = SM_RES_OK;
        if(log_out & SM_LOG_OUT_FILE) {
            char log_dpath_abs[MAX_PATH] = { 0 };
            size_t log_dpath_len = safe_strnlen(log_dpath, MAX_PATH);
            if (log_dpath_len > 0) {
                err = sm_make_path_abs(log_dpath_abs, SM_ARRAY_SIZE(log_dpath_abs), log_dpath, SM_BIN_PATH_MARKER);
                if (err != SM_RES_OK)
                    break;
            }
            size_t log_dpath_abs_len = safe_strnlen(log_dpath_abs, MAX_PATH);
            size_t log_fname_len = safe_strnlen(log_fname, MAX_PATH);
            //  init of log_config->m_log_dpath
            if (log_dpath_abs_len > 0) {
                safe_strncpy(log_config->m_log_dpath, MAX_PATH, log_dpath_abs, log_dpath_abs_len);
                sm_create_dpath(log_config->m_log_dpath);
            }
            //  init of log_config->m_log_fname
            if (log_fname_len > 0)
                safe_strncpy(log_config->m_log_fname, MAX_PATH, log_fname, log_fname_len);
            //  init local log_fpath_t
            if (log_dpath_abs_len > 0 && log_fname_len > 0) {
                if (log_config->m_log_dpath[log_dpath_abs_len - 1] == '/' || log_config->m_log_dpath[log_dpath_abs_len - 1] == '\\')
                    snprintf(log_config->m_log_fpath, SM_ARRAY_SIZE(log_config->m_log_fpath), "%s%s", log_config->m_log_dpath, log_config->m_log_fname);
                else
                    snprintf(log_config->m_log_fpath, SM_ARRAY_SIZE(log_config->m_log_fpath), "%s" SM_FILES_DIR_SLASH "%s", log_config->m_log_dpath, log_config->m_log_fname);
            }
            else if (!log_dpath_abs_len && log_fname_len > 0)
                safe_strncpy(log_config->m_log_fpath, SM_ARRAY_SIZE(log_config->m_log_fpath), log_fname, log_fname_len);
            size_t log_fpath_t_len = safe_strnlen(log_config->m_log_fpath, MAX_PATH);
            if (log_fpath_t_len > 0) {
                //  safe opening of the file in mode 'at+' , i.e.
                //  t  - text mode,
                //  a+ - opening for reading and appending
                //  file is opening in shared mode, i.e. it can be read, when log file is n't closed
                errno_t err_safe = safe_fopen(
                    &log_config->m_log_file,
                    log_config->m_log_fpath,
                    "at+",
                    SM_SFOPEN_SHARED_TYPE
                );
                err = (err_safe == SAFE_RES_OK) ? SM_RES_OK : SM_RES_ERROR;
            }
            else {
                err = SM_RES_ERROR;
                break;
            }
            if (err != SM_RES_OK)
                break;
        }
        log_config->m_log_out = log_out;
        log_config->m_min_log_level = min_log_level;
        log_config->m_max_log_level = max_log_level;
        break;
    }
#if defined SM_SYNC_LOG
    sm_sync_type_unlock(&log_config->m_sync);
#endif
    return err;
}

/*
    destroying the sm_log_config object;
    result:
        SM_RES_OK
        SM_RES_ERROR
*/
errno_t sm_log_close(sm_log_config* const log_config)
{
    errno_t res = SM_RES_ERROR;

    if (!log_config)
        return res;

#if defined SM_SYNC_LOG
    sm_sync_type_lock(&log_config->m_sync);
#endif

    //  stopping the log output
    if (log_config->m_start) {
        log_config->m_start = 0;
    }

    //  closing the log file
    if (log_config->m_log_file) {
        fclose(log_config->m_log_file);
        log_config->m_log_file = NULL;
        res = SM_RES_OK;
}

    safe_memset(log_config, sizeof(sm_log_config), 0);

#if defined SM_SYNC_LOG
    sm_sync_type_unlock(&log_config->m_sync);
    //  destoying synchronizing object
    sm_sync_type_close(&log_config->m_sync);
#endif

    return res;
}

/*
    starting the log output;
    if file isn't open, returns SM_RES_ERROR
*/
errno_t sm_log_start(sm_log_config* const log_config)
{
    errno_t res = SM_RES_ERROR;

    if (!log_config)
        return res;

#if defined SM_SYNC_LOG
    sm_sync_type_lock(&log_config->m_sync);
#endif

    if (((log_config->m_log_out & SM_LOG_OUT_FILE) && log_config->m_log_file != NULL) ||
            (log_config->m_log_out & SM_LOG_OUT_CONSOLE)) {
        log_config->m_start = 1;
        res = SM_RES_OK;
    }

#if defined SM_SYNC_LOG
    sm_sync_type_unlock(&log_config->m_sync);
#endif

    return res;
}

/*
    stopping the log output without closing of file;
    if log output isn't started, returns SM_RES_ERROR
*/
errno_t sm_log_stop(sm_log_config* const log_config)
{
    errno_t err = SM_RES_ERROR;

    if (!log_config)
        return err;

#if defined SM_SYNC_LOG
    sm_sync_type_lock(&log_config->m_sync);
#endif

    if (log_config->m_start) {
        log_config->m_start = 0;
        err = SM_RES_OK;
    }

#if defined SM_SYNC_LOG
    sm_sync_type_unlock(&log_config->m_sync);
#endif

    return err;
}

#if defined SM_OS_LINUX

/*
    multi-platform initializing the sm_log_time object;
*/
errno_t sm_log_get_local_time(sm_log_time* const time)
{
    errno_t err = SM_RES_ERROR;
    struct timeval tv_o;
    struct tm tm_o;

    if (!gettimeofday(&tv_o, NULL)) {

        localtime_r(&tv_o.tv_sec, &tm_o);

        time->m_year = tm_o.tm_year + 1900;
        time->m_month = tm_o.tm_mon;
        time->m_day_of_week = tm_o.tm_wday;
        time->m_day = tm_o.tm_mday;
        time->m_hour = tm_o.tm_hour;
        time->m_minute = tm_o.tm_min;
        time->m_second = tm_o.tm_sec;
        time->m_milliseconds = tv_o.tv_usec / 1000;

        err = SM_RES_OK;
    }
    return err;
}

#elif defined SM_OS_WINDOWS

/*
    multi-platform initializing the sm_log_time object;
*/
errno_t sm_log_get_local_time(sm_log_time* const time)
{
    SYSTEMTIME  sys_time;
    GetLocalTime(&sys_time);

    time->m_year = sys_time.wYear;
    time->m_month = sys_time.wMonth;
    time->m_day_of_week = sys_time.wDayOfWeek;
    time->m_day = sys_time.wDay;
    time->m_hour = sys_time.wHour;
    time->m_minute = sys_time.wMinute;
    time->m_second = sys_time.wSecond;
    time->m_milliseconds = sys_time.wMilliseconds;

    return SM_RES_OK;
}

#endif

/*
    log of ready text array (without formatting);

    args:
        log_config      - sm_log_config object
        log_level       - logging level
        log_category    - category of log (for filtering)
        log_text        - file name of log file
    result:
        SM_RES_OK
        SM_RES_ERROR
*/
errno_t sm_log_print(sm_log_config* const log_config,
        const unsigned int log_level,
        const char* const log_category,
        const char* const log_text)
{
    errno_t err = SM_RES_ERROR;

    if (!log_config || !log_category || !log_text || !log_config->m_start)
        return err;

    if(log_level == SM_LOG_LEVEL_OFF ||
            log_config->m_min_log_level > log_level ||
            log_config->m_max_log_level < log_level)
        return err;

    while(true) {

#if defined SM_SYNC_LOG
    sm_sync_type_lock(&log_config->m_sync);
    if (log_config->m_start != 1)
        break;
#endif

        sm_log_time time;
        sm_log_get_local_time(&time);   //  init the sm_log_time object (get current date/time)

        char log_level_str[SM_LOG_SBUFF_SIZE];

        err = sm_log_level_to_str(log_level, log_level_str, SM_ARRAY_SIZE(log_level_str));
        if(err != SM_RES_OK)
            break;

        errno_t err_file = SM_RES_OK;
        errno_t err_con = SM_RES_OK;
        if(log_config->m_log_out & SM_LOG_OUT_FILE) {
            //  printing the log info/message into the log file
            if (fprintf(log_config->m_log_file, SM_LOG_LIB_FORMAT,
                time.m_day, time.m_month, time.m_year,
                time.m_hour, time.m_minute, time.m_second, time.m_milliseconds,
                log_level_str,
                log_category ? log_category : "null",
                log_text
            ) > 0) {
                fflush(log_config->m_log_file);
                err_file = SM_RES_OK;
            }
            else
                err_file = SM_RES_ERROR;
        }
        if((log_config->m_log_out & SM_LOG_OUT_CONSOLE) &&
                log_level <= SM_LOG_LEVEL_INFO) {
            //  printing the log info/message into console
            if (printf(SM_LOG_LIB_FORMAT,
                time.m_day, time.m_month, time.m_year,
                time.m_hour, time.m_minute, time.m_second, time.m_milliseconds,
                log_level_str,
                log_category ? log_category : "null",
                log_text
            ) > 0)
                err_con = SM_RES_OK;
            else
                err_con = SM_RES_ERROR;
        }
        err = (err_file + err_con) > 0 ? SM_RES_ERROR : SM_RES_OK;
        break;
    }

#if defined SM_SYNC_LOG
    sm_sync_type_unlock(&log_config->m_sync);
#endif

    return err;
}

/*
    log of text (with formatting);

    args:
        log_config      - sm_log_config object
        log_level       - logging level
        log_category    - category of log (for filtering)
        log_format      - formatted string and suite of follow arguments
    result:
        SM_RES_OK
        SM_RES_ERROR
*/
errno_t sm_log_printf(sm_log_config* const log_config,
        const unsigned int log_level,
        const char* const log_category,
        const char* const log_format, ...)
{
    errno_t err = SM_RES_ERROR;

    if (!log_config || !log_category || !log_format || !log_config->m_start)
        return err;

    if(log_level == SM_LOG_LEVEL_OFF ||
            log_config->m_min_log_level > log_level ||
            log_config->m_max_log_level < log_level)
        return err;

    while(true) {

#if defined SM_SYNC_LOG
        sm_sync_type_lock(&log_config->m_sync);
        if (log_config->m_start != 1)
            break;
#endif

        va_list     va_args;
        char        log_buffer[SM_LOG_BUFF_SIZE];

        //  initializing log_buffer by text message from format parameters
        va_start(va_args, log_format);
        int length_res = vsnprintf(log_buffer, SM_LOG_BUFF_SIZE, log_format, va_args);
        if (length_res > 0) {
            if (length_res < SM_LOG_BUFF_SIZE)
                log_buffer[length_res] = '\0';
            else // if(length_res == _log_buffer_size)
                log_buffer[length_res - 1] = '\0';
            err = SM_RES_OK;
        }
        else
            err = SM_RES_ERROR;
        va_end(va_args);

        if (err == SM_RES_OK)
            err = sm_log_print(log_config, log_level, log_category, log_buffer);   // printing text initialized message

        break;
    }

#if defined SM_SYNC_LOG
    sm_sync_type_unlock(&log_config->m_sync);
#endif

    return err;
}

/**
 *  convert log_level value to string:

    args:
        log_level           - logging level
        log_level_str       - buffer where logging level value will be copied/printed
        log_level_str_len   - size of the buffer
    result:
        SM_RES_OK
        SM_RES_ERROR
 */
errno_t sm_log_level_to_str(const unsigned int log_level,
        char* const log_level_str,
        const size_t log_level_str_len)
{
    errno_t err = SM_RES_ERROR;
    switch(log_level) {
    case SM_LOG_LEVEL_FATAL:
        err = safe_strcpy(log_level_str, log_level_str_len, "FATAL");
        break;
    case SM_LOG_LEVEL_ERROR:
        err = safe_strcpy(log_level_str, log_level_str_len, "ERROR");
        break;
    case SM_LOG_LEVEL_WARN:
        err = safe_strcpy(log_level_str, log_level_str_len, "WARN");
        break;
    case SM_LOG_LEVEL_INFO:
        err = safe_strcpy(log_level_str, log_level_str_len, "INFO");
        break;
    case SM_LOG_LEVEL_DEBUG:
        err = safe_strcpy(log_level_str, log_level_str_len, "DEBUG");
        break;
    case SM_LOG_LEVEL_TRACE:
        err = safe_strcpy(log_level_str, log_level_str_len, "TRACE");
        break;
    default:
        err = safe_strcpy(log_level_str, log_level_str_len, "UNKNOWN");
    }
    return err;
}
