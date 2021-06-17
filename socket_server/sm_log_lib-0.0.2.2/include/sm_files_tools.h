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
        checking, if argument path is an absulute:
        i.e. 
            absolute pathes:
                "c:/some_subdir1/some_subdir_2/..."
                "d:\some_subdir1\some_subdir_2\..."
                "z:/some_subdir1\some_subdir_2/..."
                "/home/user_home/some_subdir1\some_subdir_2\..."
                returns TRUE

            relative pathes:
                "./some_subdir1/some_subdir_2/..."
                "..\some_subdir1\some_subdir_2\..."
                "./../..\.\some_subdir1\..\some_subdir_2\..."
                "some_subdir1/some_subdir_2/..."
                "some_subdir1\some_subdir_2\..."
                returns FALSE

        in args:
            path    - path, that should be analyzed/checked

        out args:
            res     - result: path is an absolute path or not (TRUE/FALSE)
    */
    errno_t sm_path_is_abs(const char* const path, BOOL* const res);

    /*
        converting, a relative path to the absolute path:
        i.e. if the current dir is the:
            "d:\current_subdir_1\current_subdir_2\current_subdir_3\"
        then:
            "./some_subdir1/some_subdir_2/..."                      ->  "d:\current_subdir_1\current_subdir_2\current_subdir_3\some_subdir1\some_subdir_2\..."
            "..\some_subdir1\some_subdir_2\..."                     ->  "d:\current_subdir_1\current_subdir_2\current_dir\some_subdir1\some_subdir_2\..."
            "./../..\.\some_subdir1\..\some_subdir_2\..."           ->  "d:\current_subdir_1\current_dir\some_subdir2\..."
            "./../../../../..\.\some_subdir1\..\some_subdir_2\..."  ->  "d:\some_subdir_2\..."
            "some_subdir1/some_subdir_2/..."                        ->  "d:\current_subdir_1\current_subdir_2\current_subdir_3\some_subdir1\some_subdir_2\..."
            "some_subdir1\some_subdir_2\..."                        ->  "d:\current_subdir_1\current_subdir_2\current_subdir_3\some_subdir1\some_subdir_2\..."

        if source path is an absolute, already make some transforming of the path, removing relative symbols:

            "d:\current_subdir_1\current_subdir_2\..\current_subdir_3\.\some_subdir1\..\some_subdir_2\..."  ->  "d:\current_subdir_1\current_subdir_3\some_subdir_2\..."

        also supports bin_path_marker in source relative path
        bin_path_marker - means, that relation pathes should be calculated/solved, using path, where binary file (current exec file sm_calc.exe) is placed:
        i.e. if the current dir is the:
            "d:\current_subdir_1\current_subdir_2\current_subdir_3\"
        if current path of sm_calc binary is follow:
            c:\bin\sm_calculator\bin\sm_calc.exe
            then base directory path is:
            c:\bin\sm_calculator\bin\
        and if 
            bin_path_marker == *{SM_CALC_BIN}*
        and then:
            "*{SM_CALC_BIN}/some_subdir1/some_subdir_2/..."                     ->  "c:\bin\sm_calculator\bin\some_subdir1\some_subdir_2\..."
            "*{SM_CALC_BIN}/..\some_subdir1\some_subdir_2\..."                  ->  "c:\bin\sm_calculator\some_subdir1\some_subdir_2\..."
            "*{SM_CALC_BIN}\some_subdir1\some_subdir_2/..."                     ->  "c:\bin\sm_calculator\some_subdir1\some_subdir_2\..."

        On Linux platfm, instead "c:\" and "\" will be used "/" (as root) and "/" as delimeter

        in args:
            path            - path, that should be analyzed/checked
            bin_path_marker - marker, that if 
            path_abs_len    - length of result buffer

        out args:
            path_abs        - result absolute path will be placed here
    */
    errno_t sm_make_path_abs(char* const path_abs, const size_t path_abs_len, const char* const path, const char* const bin_path_marker);

    /*
        return current directory path:

        in args:
            dpath_len       - length of buffer, where result should be copied

        out args:
            dpath           - buffer, where directory path should be copied
    */
    errno_t sm_get_cur_dpath(char* const dpath, const size_t dpath_len);
    
    /*
        checking, if is some directory exists
        returns 
            SM_RES_ERROR and res == FALSE   - if dpath doesn't exist
            SM_RES_OK and res == FALSE      - if dpath exist, but this is not a directory
            SM_RES_OK and res == TRUE       - if dpath exist, and this is a directory

        in args:
            dpath           - source directory

        out args:
            res             - boolean, where result will be saved
    */
    errno_t sm_is_dpath_found(const char* const dpath, BOOL* res);

    /*
        creates directory

        in args:
            dpath           - source directory
    */
    errno_t sm_create_dpath(const char* const dpath);

    /*
        returns path of main module of executable file
        i.e.:
            c:\bin\sm_calculator\bin\sm_calc.exe
            /home/user_hone/bin/sm_calculator/bin/sm_calc

        in args:
            exe_fpath_len       - length of buffer, where main module will be placed

        out args:
            exe_fpath           - result absolute path of main module will be placed here
    */
    errno_t sm_get_base_module_fpath(char* const exe_fpath, const size_t exe_fpath_len);

    /*
        returns directory path from full path:
        i.e.:
            c:\bin\sm_calculator\bin\sm_calc.exe            ->  c:\bin\sm_calculator\bin\
            /home/user_hone/bin/sm_calculator/bin/sm_calc   ->  /home/user_hone/bin/sm_calculator/bin/

        in args:
            dpath           - buffer, where directory path will be saved 
            dpath_len       - length of the buffer, where directory path will be saved

        out args:
            fpath           - source file path
    */
    errno_t sm_get_dpath_from_fpath(char* const dpath, const size_t dpath_len, const char* const fpath);

    /*
        returns file name from full path:
        i.e.:
            c:\bin\sm_calculator\bin\sm_calc.exe            ->  sm_calc.exe
            /home/user_hone/bin/sm_calculator/bin/sm_calc   ->  sm_calc

        in args:
            fname           - buffer, where file name will be saved
            fname_len       - length of the buffer, where file name will be saved

        out args:
            fpath           - source file path
    */
    errno_t sm_get_fname_from_fpath(char* const fname, const size_t fname_len, const char* const fpath);

    /*
        returns temp directory path:

        in args:
            dpath_len       - length of the buffer, where temp directory path will be saved

        out args:
            dpath           - buffer, where temp directory path will be saved
    */
    errno_t sm_get_temp_dpath(char* const dpath, const size_t dpath_len);

#if defined __cplusplus
}
#endif
