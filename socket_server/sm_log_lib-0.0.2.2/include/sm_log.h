/*
 *    Copyright (c) 2020-2021 SManSoft <http://www.smansoft.com/>
 *    Sergey Manoylo <info@smansoft.com>
 */

#pragma once

#include "pch.h"

#include "sm_log_types.h"

#if defined __cplusplus
extern "C" {
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
            const unsigned int min_log_level,
            const unsigned int max_log_level,
            const char* const log_fpath);

    /*
        initializing the sm_log_config object, using two argumets: directory path and log file name;
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
            const unsigned int min_log_level,
            const unsigned int max_log_level,
            const char* const log_dpath, const char* const log_fname);

    /*
        destroying the sm_log_config object;
        result:
            SM_RES_OK
            SM_RES_ERROR
    */
    errno_t sm_log_close(sm_log_config* const log_config);

    /*
        starting the log output;
        if file isn't open, returns SM_RES_ERROR
    */
    errno_t sm_log_start(sm_log_config* const log_config);

    /*
        stopping the log output without closing of file;
        if log output isn't started, returns SM_RES_ERROR
    */
    errno_t sm_log_stop(sm_log_config* const log_config);

    /*
        multi-platform initializing the sm_log_time object;
    */
    errno_t sm_log_get_local_time(sm_log_time* const time);

    /*
        log of ready text array (without formatting);

        args:
            log_config      - sm_log_config object
            log_level       - logging level
            log_category    - category of log (for filtering)
            log_text        - file name of log file
    */
    errno_t sm_log_print(sm_log_config* const log_config,
            const unsigned int log_level,
            const char* const log_category,
            const char* const log_text);

    /*
        log of text (with formatting);

        args:
            log_config      - sm_log_config object
            log_level       - logging level
            log_category    - category of log (for filtering)
            log_format      - formatted string and suite of follow arguments
    */
    errno_t sm_log_printf(sm_log_config* const log_config,
            const unsigned int log_level,
            const char* const log_category,
            const char* const log_format, ...);

    /*
        convert log_level value to string;

        args:
            log_level           - logging level
            log_level_str       - buffer where logging level value will be copied/printed
            log_level_str_len   - size of the buffer
    */
    errno_t sm_log_level_to_str(const unsigned int log_level,
            char* const log_level_str,
            const size_t log_level_str_len);

#if defined __cplusplus
}
#endif
