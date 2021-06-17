/*
 *    Copyright (c) 2020-2021 SManSoft <http://www.smansoft.com/>
 *    Sergey Manoylo <info@smansoft.com>
 */

#pragma once

#include "safe_types.h"

#if defined __cplusplus
extern "C" {
#endif

/*
    multiplaform secure reading a line from stdin file stream;
    args:
        stream  - stream, which is source of read data
        str	    - buffer, that should be filled by read data from stdin file stream
        len	    - size of the buffer
*/
char* safe_gets(FILE* const stream, char* const str, const size_t len);

#ifdef __cplusplus
}
#endif





