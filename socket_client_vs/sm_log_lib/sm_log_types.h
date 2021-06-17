/*
 *    Copyright (c) 2020-2021 SManSoft <http://www.smansoft.com/>
 *    Sergey Manoylo <info@smansoft.com>
 */

#pragma once

#include "pch.h"

#if defined __cplusplus
extern "C" {
#endif
    
#ifndef SM_LOG_VERSION
#define SM_LOG_VERSION    "0.0.2.2"
#endif

typedef int             BOOL;           // type BOOL definition
typedef int             errno_t;        // definition of errno_t (int)
                                        // errno_t - is a basic type, returned by log output functions

#ifndef NULL
#define NULL            0
#endif

#ifndef VOID
#define VOID            void
#endif

#ifndef FALSE
#define FALSE           0
#endif

#ifndef TRUE
#define TRUE            1
#endif

#if !defined MAX_PATH
#define MAX_PATH    260
#endif

#if !defined SM_ARRAY_SIZE
#define SM_ARRAY_SIZE(in_array) (sizeof(in_array)/sizeof(in_array[0]))  //  calc size of the array
#endif

#if defined SM_OS_LINUX
#define SM_FILES_DIR_SLASH      "/"
#elif defined SM_OS_WINDOWS
#define SM_FILES_DIR_SLASH      "\\"
#endif

#ifndef SM_RES_OK
#define SM_RES_OK           0
#endif

#ifndef SM_RES_ERROR
#define SM_RES_ERROR        1
#endif

#ifndef SM_BIN_PATH_MARKER
#define SM_BIN_PATH_MARKER      "*{SM_CALC_BIN}*"   //  bin_path_marker - means, that relation pathes should be calculated/solved, using path,
                                                        //  where binary file (current exec file sm_calc.exe) is placed instead
                                                        //  current directory path
#endif

#define SM_LOG_OUT_FILE         1
#define SM_LOG_OUT_CONSOLE      2

#define SM_LOG_LEVEL_OFF        0
#define SM_LOG_LEVEL_FATAL      10
#define SM_LOG_LEVEL_ERROR      20
#define SM_LOG_LEVEL_WARN       30
#define SM_LOG_LEVEL_INFO       40
#define SM_LOG_LEVEL_DEBUG      50
#define SM_LOG_LEVEL_TRACE      60
#define SM_LOG_LEVEL_ALL        SM_LOG_LEVEL_TRACE

#if defined SM_SYNC_LOG

    /*
        multi-platform log synchronizing type;

        as a rule (if a developer don't use non-synchronized run-time in VS),
        multi-platform log synchronizing type don't need;

        if developer use multi-thread run-time in VS, all output to file will be synchronized;

        this log synchronizing type can be used if developer need to synchronize several log outs:

        sm_log_config log_config;

        ...

        sm_sync_type_lock(&log_config.m_sync);

        sm_log_print(&log_config, "some categorty", "log message 1");
        sm_log_print(&log_config, "some categorty", "log message 2");
        sm_log_print(&log_config, "some categorty", "log message 3");

        sm_sync_type_unlock(&log_config.m_sync);

        In this case all calls of sm_log_print between sm_sync_type_lock() and
        sm_sync_type_unlock() should be thread-synchronized;
    */
    typedef struct _sm_sync_type
    {
#if defined SM_OS_LINUX
        pthread_mutex_t     m_sync; // sync object (mutex) (Linux)
#elif defined SM_OS_WINDOWS
        CRITICAL_SECTION    m_sync; // sync object (critical section) (Windows)
#endif
    } sm_sync_type, * psm_sync_type;

    /*  initializing the multi-platform synchronizing object    */
    errno_t sm_sync_type_init(sm_sync_type* const sync_type);

    /*  destroying the multi-platform synchronizing object  */
    errno_t sm_sync_type_close(sm_sync_type* const sync_type);

    /*  locking the current thread, using multi-platform synchronizing object   */
    errno_t sm_sync_type_lock(sm_sync_type* const sync_type);

    /*  checking for lock the current thread, using multi-platform synchronizing object   */
    errno_t sm_sync_type_try_lock(sm_sync_type* const sync_type);

    /*  unlocking the current thread, using multi-platform synchronizing object   */
    errno_t sm_sync_type_unlock(sm_sync_type* const sync_type);

#endif



    /*
        internal type for saving of time data;
    */
    typedef struct _sm_log_time
    {
        unsigned int m_year;
        unsigned int m_month;
        unsigned int m_day_of_week;
        unsigned int m_day;
        unsigned int m_hour;
        unsigned int m_minute;
        unsigned int m_second;
        unsigned int m_milliseconds;
    } sm_log_time, * psm_log_time;

    /*
        main log support structure;
        contains all necessary info for control/providing the log output;
        supports simple log output without rolling;
        supports synchronizing (requires preprocessor definition: SM_SYNC_LOG);
    */
    typedef struct _sm_log_config
    {
        unsigned int    m_start;    //  starting flag:   if m_start == 0 - even if log file is opened,
                                    //                                      log functions will not be written in the log file
                                    //                   if m_start == 1 - if log file is opened,
                                    //                                      log functions will not be written in the log file
        char            m_log_dpath[MAX_PATH];  //  directory, where the log file will be created
        char            m_log_fname[MAX_PATH];  //  file name of the log file
        char            m_log_fpath[MAX_PATH];  //  full path of the flog file 

#if defined SM_SYNC_LOG
        sm_sync_type    m_sync;         //  multi-platform synchronized object
#endif

        unsigned int                m_min_log_level;    // minimum log level value
        unsigned int                m_max_log_level;    // maximum log level value

        unsigned int                m_log_out;

        FILE* m_log_file;    //  log file object, where all log will be printed
    } sm_log_config, * psm_log_config;


#if defined __cplusplus
}
#endif

