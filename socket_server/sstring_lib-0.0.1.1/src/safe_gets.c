/*
 *    Copyright (c) 2020-2021 SManSoft <http://www.smansoft.com/>
 *    Sergey Manoylo <info@smansoft.com>
 */

#include "safe_gets.h"
#include "safe_str_lib.h"

/*
    multiplaform secure reading a line from stdin file stream;
    args:
        stream  - stream, which is source of read data
        str	    - buffer, that should be filled by read data from stdin file stream
		len	    - size of the buffer
*/
char* safe_gets(FILE* const stream, char* const str, const size_t len)
{
    char* ret = fgets(str, (int)len, stream ? stream : stdin);
    if (ret) {
        size_t last_ch = safe_strnlen(str, len) - 1;
        if (str[last_ch] == '\n')
            str[last_ch] = '\0';
    }
    return ret;
}
