/*
 *    Copyright (c) 2020-2021 SManSoft <http://www.smansoft.com/>
 *    Sergey Manoylo <info@smansoft.com>
 */

#include "pch.h"

#include "safe_str_lib.h"

#include "sm_files_tools.h"

#if !defined SM_MAX_SUBDIRS
#define SM_MAX_SUBDIRS 50
#endif

#if defined SM_OS_LINUX

/*
    return current directory path:

    in args:
        dpath_len       - length of buffer, where result should be copied

    out args:
        dpath           - buffer, where directory path should be copied
*/
errno_t sm_get_cur_dpath(char* const dpath, const size_t dpath_len)
{
    errno_t err;
    char* res =  getcwd(dpath, dpath_len - 1);
    if (res)
        err = SM_RES_OK;
    else
        err = SM_RES_ERROR;
    return err;
}

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
errno_t sm_is_dpath_found(const char* const dpath, BOOL* res)
{
    errno_t err = SM_RES_ERROR;
    struct stat st;
    if (!stat(dpath, &st)) {
        *res = S_ISDIR(st.st_mode) ? TRUE : FALSE;
        err = SM_RES_OK;
    }
    else {
        *res = FALSE;
        err = SM_RES_ERROR;
    }
    return err;
}

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
errno_t sm_get_base_module_fpath(char* const exe_fpath, const size_t exe_fpath_len)
{
    errno_t err = SM_RES_ERROR;
    const char curr_proc_exe[] = "/proc/self/exe";
    if(!exe_fpath_len)
        return err;
    exe_fpath[0] = '\0';
    size_t len = (size_t)readlink(curr_proc_exe, exe_fpath, exe_fpath_len - 1);
    if (len > 0) {
        exe_fpath[len] = '\0';
        err = SM_RES_OK;
    }
    return err;
}

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
errno_t sm_path_is_abs(const char* const path, BOOL* const res)
{
    errno_t err = SM_RES_ERROR;
    size_t path_len = safe_strnlen(path, MAX_PATH);
    if (path_len >= 1) {
        *res = (path[0] == '/') ? TRUE : FALSE;
        err = SM_RES_OK;
    }
    return err;
}

/*
    returns temp directory path:

    in args:
        dpath           - buffer, where temp directory path will be saved

    out args:
        dpath_len       - length of the buffer, where temp directory path will be saved
*/
errno_t sm_get_temp_dpath(char* const dpath, const size_t dpath_len)
{
    errno_t err_safe;
    char* res_env_v = NULL;
    while (TRUE) {
        res_env_v = getenv("TMPDIR");
        if (res_env_v)
            break;
        res_env_v = getenv("TMP");
        if (res_env_v)
            break;
        res_env_v = getenv("TEMP");
        if (res_env_v)
            break;
        res_env_v = getenv("TEMPDIR");
        break;
    }
    if (res_env_v) {
        err_safe = safe_strcpy(dpath, dpath_len, res_env_v);
        if (err_safe)
            return SM_RES_ERROR;
    }
    else {
        err_safe = safe_strcpy(dpath, dpath_len, "/tmp");
        if (err_safe)
            return SM_RES_ERROR;
    }
    return SM_RES_OK;
}

#elif defined SM_OS_WINDOWS

/*
    return current directory path:

    in args:
        dpath_len       - length of buffer, where result should be copied

    out args:
        dpath           - buffer, where directory path should be copied
*/
errno_t sm_get_cur_dpath(char* const dpath, const size_t dpath_len)
{
    errno_t err;
    DWORD res_len = GetCurrentDirectoryA((DWORD)(dpath_len - 1), dpath);
    if (res_len > 0) {
        dpath[res_len] = '\0';
        err = SM_RES_OK;
    }
    else
        err = SM_RES_ERROR;
    return err;
}

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
errno_t sm_is_dpath_found(const char* const dpath, BOOL* res)
{
    errno_t err;
    DWORD dir_attr = GetFileAttributesA(dpath);
    if (dir_attr != INVALID_FILE_ATTRIBUTES) {
        *res = ((dir_attr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY);
        err = SM_RES_OK;
    }
    else {
        *res = FALSE;
        err = SM_RES_ERROR;
    }
    return err;
}

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
errno_t sm_get_base_module_fpath(char* const exe_fpath, const size_t exe_fpath_len)
{
    errno_t err = SM_RES_ERROR;
    DWORD num_copied = GetModuleFileNameA(NULL, exe_fpath, (DWORD)(exe_fpath_len - 1));
    if (num_copied > 0) {
        exe_fpath[num_copied] = '\0';
        err = SAFE_RES_OK;
    }
    return err;
}

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
errno_t sm_path_is_abs(const char* const path, BOOL* const res)
{
    errno_t err = SM_RES_ERROR;
    size_t path_len = safe_strnlen(path, MAX_PATH);
    if (path_len >=2) {
        *res = ((path[0] >= 'a' && path[0] <= 'z' || path[0] >= 'A' && path[0] <= 'Z') && path[1] == ':') ? TRUE : FALSE;
        err = SM_RES_OK;
    }
    return err;
}

/*
    returns temp directory path:

    in args:
        dpath_len       - length of the buffer, where temp directory path will be saved

    out args:
        dpath           - buffer, where temp directory path will be saved

*/
errno_t sm_get_temp_dpath(char* const dpath, const size_t dpath_len)
{
    errno_t err = SM_RES_ERROR;
    DWORD len = GetTempPathA((DWORD)dpath_len-1, dpath);
    if (len > 0) {
        dpath[len] = '\0';
        err = SM_RES_OK;
    }
    return err;
}

#endif

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
errno_t sm_get_dpath_from_fpath(char* const dpath, const size_t dpath_len, const char* const fpath)
{
    errno_t err = SM_RES_ERROR;
    errno_t err_safe = SM_RES_ERROR;
    char fpath_w[MAX_PATH] = { 0 };

    char* first_slash = NULL;
    char* first_back_slash = NULL;
    char* first_res = NULL;

    size_t fpath_len = safe_strnlen(fpath, MAX_PATH);
    if (fpath_len <= 0)
        return err;

    err_safe = safe_strncpy(fpath_w, SM_ARRAY_SIZE(fpath_w), fpath, fpath_len);
    if (err_safe)
        return SM_RES_ERROR;

    size_t fpath_w_len = safe_strnlen(fpath_w, MAX_PATH);
    if (fpath_w_len <= 0)
        return SM_RES_ERROR;

    err_safe = safe_strlastchar(fpath_w, fpath_w_len, '\\', &first_back_slash);
    err_safe = safe_strlastchar(fpath_w, fpath_w_len, '/', &first_slash);

    first_res = (first_back_slash > first_slash) ? first_back_slash : first_slash;
    if (first_res) {
        *(first_res+1) = '\0';

        fpath_w_len = safe_strnlen(fpath_w, MAX_PATH);
        if (fpath_w_len <= 0)
            return SM_RES_ERROR;

        err_safe = safe_strncpy(dpath, dpath_len, fpath_w, fpath_w_len);
        if (err_safe)
            return SM_RES_ERROR;
    }
    else if (dpath_len > 0) {
#if defined SM_OS_WINDOWS
        if (fpath_w_len == 2) {
            if (((fpath_w[0] >= 'a' && fpath_w[0] <= 'z' || fpath_w[0] >= 'A' && fpath_w[0] <= 'Z') && fpath_w[1] == ':')) {
                err_safe = safe_strncat(fpath_w, SM_ARRAY_SIZE(fpath_w), SM_FILES_DIR_SLASH, SM_ARRAY_SIZE(SM_FILES_DIR_SLASH));
                if (err_safe)
                    return SM_RES_ERROR;
                fpath_w_len = safe_strnlen(fpath_w, MAX_PATH);
                if (fpath_w_len <= 0)
                    return SM_RES_ERROR;
                err_safe = safe_strncpy(dpath, dpath_len, fpath_w, fpath_w_len);
                if (err_safe)
                    return SM_RES_ERROR;
            }
        }
        else
            *dpath = '\0';
#elif defined SM_OS_LINUX
        *dpath = '\0';
#endif
    }
    else
        return SM_RES_ERROR;

    return SM_RES_OK;
}

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
errno_t sm_get_fname_from_fpath(char* const fname, const size_t fname_len, const char* const fpath)
{
    errno_t err = SM_RES_ERROR;

    errno_t err_safe = SM_RES_ERROR;
    char fpath_w[MAX_PATH] = { 0 };
    char fname_w[MAX_PATH] = { 0 };

    char* first_slash = NULL;
    char* first_back_slash = NULL;
    char* first_res = NULL;

    size_t fpath_len = safe_strnlen(fpath, MAX_PATH);
    if (fpath_len <= 0)
        return err;

    err_safe = safe_strncpy(fpath_w, SM_ARRAY_SIZE(fpath_w), fpath, fpath_len);
    if (err_safe)
        return SM_RES_ERROR;

    size_t fpath_w_len = safe_strnlen(fpath_w, MAX_PATH);
    if (fpath_w_len <= 0)
        return SM_RES_ERROR;

    err_safe = safe_strlastchar(fpath_w, fpath_w_len, '\\', &first_back_slash);
    err_safe = safe_strlastchar(fpath_w, fpath_w_len, '/', &first_slash);

    first_res = (first_back_slash > first_slash) ? first_back_slash : first_slash;
    if (first_res) {
        first_res++;
        err_safe = safe_strcpy(fname_w, SM_ARRAY_SIZE(fname_w), first_res);
        if (err_safe)
            return SM_RES_ERROR;

        size_t fname_w_len = safe_strnlen(fname_w, MAX_PATH);
        if (fname_w_len <= 0)
            return SM_RES_ERROR;

        err_safe = safe_strncpy(fname, fname_len, fname_w, fname_w_len);
        if (err_safe)
            return SM_RES_ERROR;
    }
    else {
        fpath_w_len = safe_strnlen(fpath_w, MAX_PATH);
        if (fpath_w_len <= 0)
            return SM_RES_ERROR;
#if defined SM_OS_WINDOWS
        if (fpath_w_len == 2) {
            if (((fpath_w[0] >= 'a' && fpath_w[0] <= 'z' || fpath_w[0] >= 'A' && fpath_w[0] <= 'Z') && fpath_w[1] == ':'))
                return SM_RES_ERROR;
        }
#endif
        err_safe = safe_strncpy(fname, fname_len, fpath_w, fpath_w_len);
        if (err_safe)
            return SM_RES_ERROR;
    }
    return SM_RES_OK;
}

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
errno_t sm_make_path_abs(char* const path_abs, const size_t path_abs_len, const char* const path, const char* const bin_path_marker)
{
    errno_t err = SM_RES_ERROR;
    errno_t err_safe;
    char path_rel_w[MAX_PATH] = { 0 };
    char path_cur[MAX_PATH] = { 0 };
    size_t path_cur_len;

    BOOL is_file;
    BOOL is_abs;
    char* substr;

    size_t path_len = safe_strnlen(path, MAX_PATH);
    if (path_len <= 0)
        return SM_RES_ERROR;

    is_file = (path[path_len - 1] != '\\' && path[path_len - 1] != '/');

    err = sm_path_is_abs(path, &is_abs);
    if (err != SM_RES_OK)
        return err;

    if (!is_abs) {
        err_safe = safe_strncpy(path_rel_w, SM_ARRAY_SIZE(path_rel_w), path, path_len);
        if (err_safe)
            return SM_RES_ERROR;

        size_t path_rel_w_len = safe_strnlen(path_rel_w, MAX_PATH);
        if (path_rel_w_len <= 0)
            return SM_RES_ERROR;

        size_t bin_path_marker_len = safe_strnlen(bin_path_marker, MAX_PATH);

        errno_t err_strstr = 1;
        substr = NULL;

        if (bin_path_marker_len > 0)
            err_strstr = safe_strstr(path_rel_w, path_rel_w_len, bin_path_marker, bin_path_marker_len, &substr);
        if (!err_strstr && substr && (substr == path_rel_w)) {
            char exe_fpath[MAX_PATH] = { 0 };
            char* substr_dir = NULL;
            //  bin_path_relation
            err = sm_get_base_module_fpath(exe_fpath, SM_ARRAY_SIZE(exe_fpath));
            if (err != SM_RES_OK)
                return err;
            err = sm_get_dpath_from_fpath(path_cur, SM_ARRAY_SIZE(path_cur), exe_fpath);
            if (err != SM_RES_OK)
                return err;
            path_cur_len = safe_strnlen(path_cur, MAX_PATH);
            if (path_cur_len <= 0)
                return SM_RES_ERROR;

            substr_dir = substr + bin_path_marker_len;
            size_t substr_dir_len = safe_strnlen(substr_dir, MAX_PATH);

            if (substr_dir_len > 0 && (path_cur[path_cur_len - 1] == '\\' || path_cur[path_cur_len - 1] == '/') &&
                (substr_dir[0] == '\\' || substr_dir[0] == '/')) {
                substr_dir++;
                substr_dir_len = safe_strnlen(substr_dir, MAX_PATH);
            }
            if (!substr_dir_len && path_cur[path_cur_len - 1] != '\\' && path_cur[path_cur_len - 1] == '/' ||
                substr_dir_len > 0 && path_cur[path_cur_len - 1] != '\\' && path_cur[path_cur_len - 1] != '/' &&
                substr_dir[0] != '\\' && substr_dir[0] != '/') {
                err_safe = safe_strncat(path_cur, SM_ARRAY_SIZE(path_cur), SM_FILES_DIR_SLASH, SM_ARRAY_SIZE(SM_FILES_DIR_SLASH));
                if (err_safe)
                    return SM_RES_ERROR;
            }
            if (substr_dir_len > 0) {
                err_safe = safe_strncat(path_cur, SM_ARRAY_SIZE(path_cur), substr_dir, substr_dir_len);
                if (err_safe)
                    return SM_RES_ERROR;
            }
        }
        else {
            //  curr_path_relation
            err = sm_get_cur_dpath(path_cur, SM_ARRAY_SIZE(path_cur));
            if (err != SM_RES_OK)
                return err;
            path_cur_len = safe_strnlen(path_cur, MAX_PATH);
            if (path_cur_len <= 0)
                return SM_RES_ERROR;
            if (path_cur[path_cur_len - 1] != '\\' && path_cur[path_cur_len - 1] != '/') {
                err_safe = safe_strncat(path_cur, SM_ARRAY_SIZE(path_cur), SM_FILES_DIR_SLASH, SM_ARRAY_SIZE(SM_FILES_DIR_SLASH));
                if (err_safe)
                    return SM_RES_ERROR;
            }
            err_safe = safe_strncat(path_cur, SM_ARRAY_SIZE(path_cur), path, path_len);
            if (err_safe)
                return SM_RES_ERROR;
        }
    }
    else {
        err_safe = safe_strncpy(path_cur, SM_ARRAY_SIZE(path_cur), path, path_len);
        if (err_safe)
            return SM_RES_ERROR;
    }

    path_cur_len = safe_strnlen(path_cur, MAX_PATH);
    if (path_cur_len <= 0)
        return err;

    char path_cur_w[MAX_PATH] = { 0 };
    err_safe = safe_strncpy(path_cur_w, SM_ARRAY_SIZE(path_cur_w), path_cur, path_cur_len);
    if (err_safe)
        return SM_RES_ERROR;

    size_t path_cur_w_len = safe_strnlen(path_cur_w, MAX_PATH);
    if (path_cur_w_len <= 0)
        return SM_RES_ERROR;

    char sub_dirs[SM_MAX_SUBDIRS][MAX_PATH] = { 0 };        //  contains all subdirs
    char sub_dirs_proc[MAX_PATH] = { 0 };                   //  in cycle - generation of full path from subdirs 

    char* ptr;
    char* res;

    int count = 0;

    BOOL is_cur_dir;
    BOOL is_par_dir;

    size_t res_len;

    err_safe = 0;
    if (path_cur_w[0] == '/')
        sub_dirs[count++][0] = '\0';
    if(path_cur_w_len)
        res = safe_strtok(path_cur_w, &path_cur_w_len, "\\/", &ptr);
    else
        res = NULL;
    if (res) {
        res_len = safe_strnlen(res, MAX_PATH);
        is_cur_dir = (res_len == 1 && res[0] == '.') ? TRUE : FALSE;
        is_par_dir = (res_len == 2 && res[0] == '.' && res[1] == '.') ? TRUE : FALSE;
        if (!is_cur_dir && !is_par_dir)
            err_safe = safe_strncpy(sub_dirs[count++], MAX_PATH, res, res_len);
    }
    while (count < SM_MAX_SUBDIRS && res != NULL && !err_safe) {
        if(path_cur_w_len)
            res = safe_strtok(NULL, &path_cur_w_len, "\\/", &ptr);
        else
            res = NULL;
        if (res) {
            res_len = safe_strnlen(res, MAX_PATH);
            is_cur_dir = (res_len == 1 && res[0] == '.') ? TRUE : FALSE;
            is_par_dir = (res_len == 2 && res[0] == '.' && res[1] == '.') ? TRUE : FALSE;
            if (!is_cur_dir && !is_par_dir)
                err_safe = safe_strncpy(sub_dirs[count++], MAX_PATH, res, res_len);
            else if (is_cur_dir)
                continue;
            else if (is_par_dir) {
                if (count > 1) {
                    --count;
                    sub_dirs[count][0] = '\0';
                }
                continue;
            }
        }
    }
    err = count > 0 ? SM_RES_OK : SM_RES_ERROR;
    if (err == SM_RES_OK) {
        sub_dirs_proc[0] = '\0';
        for (int i = 0; i < count; i++) {
            size_t sub_dir_len = safe_strnlen(sub_dirs[i], MAX_PATH);
            if (sub_dir_len > 0) {
                err_safe = safe_strncat(sub_dirs_proc, MAX_PATH, sub_dirs[i], sub_dir_len);
                if (err_safe) {
                    err = SM_RES_ERROR;
                    break;
                }
            }
            if (!is_file || i != (count - 1)) {
                err_safe = safe_strncat(sub_dirs_proc, MAX_PATH, SM_FILES_DIR_SLASH, SM_ARRAY_SIZE(SM_FILES_DIR_SLASH));
                if (err_safe) {
                    err = SM_RES_ERROR;
                    break;
                }
            }
        }
    }

    size_t sub_dirs_proc_len = safe_strnlen(sub_dirs_proc, MAX_PATH);
    if (sub_dirs_proc_len <= 0)
        return SM_RES_ERROR;

    err_safe = safe_strncpy(path_abs, path_abs_len, sub_dirs_proc, sub_dirs_proc_len);
    if (err_safe)
        return SM_RES_ERROR;
    return SM_RES_OK;
}

/*
    creates directory

    in args:
        dpath           - source directory
*/
errno_t sm_create_dpath(const char* const dpath)
{
    errno_t err = SM_RES_ERROR;
    errno_t err_safe;
    char dpath_w[MAX_PATH] = { 0 };                         //  working path (copy of dir_path) for safe_strtok
    char sub_dirs[SM_MAX_SUBDIRS][MAX_PATH] = { 0 };        //  contains all subdirs
    char sub_dirs_proc[MAX_PATH] = { 0 };                   //  in cycle - generation of full path from subdirs 
    char* ptr;
    char* res;
    int count = 0;
    size_t res_len;
    BOOL is_cur_dir;
    BOOL is_par_dir;

    size_t dpath_len = safe_strnlen(dpath, MAX_PATH);
    if (dpath_len <= 0)
        return SM_RES_ERROR;

    err = sm_make_path_abs(dpath_w, SM_ARRAY_SIZE(dpath_w), dpath, SM_BIN_PATH_MARKER);
    if (err != SM_RES_OK)
        return err;

    size_t dpath_w_len = safe_strnlen(dpath_w, MAX_PATH);
    if (dpath_w_len <= 0)
        return SM_RES_ERROR;

    err_safe = 0;
#if defined SM_OS_LINUX
    if (dpath_w[0] == '/')
        sub_dirs[count++][0] = '\0';
#endif
    if(dpath_w_len)
        res = safe_strtok(dpath_w, &dpath_w_len, "\\/", &ptr);
    else
        res = NULL;
    if (res) {
        res_len = safe_strnlen(res, MAX_PATH);
        is_cur_dir = (res_len == 1 && res[0] == '.') ? TRUE : FALSE;
        is_par_dir = (res_len == 2 && res[0] == '.' && res[1] == '.') ? TRUE : FALSE;
        if(!is_cur_dir && !is_par_dir) 
            err_safe = safe_strncpy(sub_dirs[count++], MAX_PATH, res, res_len);
    }
    while (count < SM_MAX_SUBDIRS && res != NULL && !err_safe) {
        if(dpath_w_len)
            res = safe_strtok(NULL, &dpath_w_len, "\\/", &ptr);
        else
            res = NULL;
        if (res) {
            res_len = safe_strnlen(res, MAX_PATH);
            is_cur_dir = (res_len == 1 && res[0] == '.') ? TRUE : FALSE;
            is_par_dir = (res_len == 2 && res[0] == '.' && res[1] == '.') ? TRUE : FALSE;
            if (!is_cur_dir && !is_par_dir)
                err_safe = safe_strncpy(sub_dirs[count++], MAX_PATH, res, res_len);
            else if (is_cur_dir)
                continue;
            else if (is_par_dir) {
                if (count > 1) {
                    --count;
                    sub_dirs[count][0] = '\0';
                }
                continue;
            }
        }
    }
    err = count > 0 ? SM_RES_OK : SM_RES_ERROR;
    if (err == SM_RES_OK) {
        sub_dirs_proc[0] = '\0';
        BOOL is_dir;
        errno_t err_tmp;
        for (int i = 0; i < count; i++) {
            size_t sub_dir_len = safe_strnlen(sub_dirs[i], MAX_PATH);
            if (sub_dir_len > 0) {
                err_safe = safe_strncat(sub_dirs_proc, MAX_PATH, sub_dirs[i], sub_dir_len);
                if (err_safe) {
                    err = SM_RES_ERROR;
                    break;
                }
            }
            err_safe = safe_strncat(sub_dirs_proc, MAX_PATH, SM_FILES_DIR_SLASH, SM_ARRAY_SIZE(SM_FILES_DIR_SLASH));
            if (err_safe) {
                err = SM_RES_ERROR;
                break;
            }
            err_tmp = sm_is_dpath_found(sub_dirs_proc, &is_dir);    //  if this directory of subdirectory extists - don't create it 
            if (err_tmp == SM_RES_ERROR && !is_dir) {   // if dpath doesn't exists - create directory
#if defined SM_OS_LINUX
                mkdir(sub_dirs_proc, S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);  // attributes: 755 (rwx,r-x,r-x)
#elif defined SM_OS_WINDOWS
                CreateDirectoryA(sub_dirs_proc, NULL);
#endif
            }
            else if (err_tmp == SM_RES_OK && !is_dir) { // if dpath exists, but this isn't a directory - it's the error
                err = SM_RES_OK;
                break;
            }
        }
    }
    return err;
}
