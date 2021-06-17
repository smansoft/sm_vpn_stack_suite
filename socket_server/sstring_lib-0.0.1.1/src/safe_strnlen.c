/*------------------------------------------------------------------
 * safe_strnlen.c
 *
 * October 2008, Bo Berry
 *
 * Copyright (c) 2008-2011 by Cisco Systems, Inc
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *------------------------------------------------------------------
 */

#include "safe_clib_private.h"
#include "safe_str_constraint.h"
#include "safe_str_lib.h"


/**
 * NAME
 *    safe_strnlen
 *
 * SYNOPSIS
 *    #include "safe_str_lib.h"
 *    rsize_t
 *    safe_strnlen(const char *dest, rsize_t dmax)
 *
 * DESCRIPTION
 *    The safe_strnlen function computes the length of the string pointed
 *    to by dest.
 *
 * SPECIFIED IN
 *    ISO/IEC TR 24731-1, Programming languages, environments
 *    and system software interfaces, Extensions to the C Library,
 *    Part I: Bounds-checking interfaces
 *
 * INPUT PARAMETERS
 *    dest      pointer to string
 *
 *    dmax      restricted maximum length.
 *
 * OUTPUT PARAMETERS
 *    none
 *
 * RUNTIME CONSTRAINTS
 *    dest shall not be a null pointer
 *    dmax shall not be greater than RSIZE_MAX_STR
 *    dmax shall not equal zero
 *
 * RETURN VALUE
 *    The function returns the string length, excluding  the terminating
 *    null character.  If dest is NULL, then safe_strnlen returns 0.
 *
 *    Otherwise, the safe_strnlen function returns the number of characters
 *    that precede the terminating null character. If there is no null
 *    character in the first dmax characters of dest then safe_strnlen returns
 *    dmax. At most the first dmax characters of dest are accessed
 *    by safe_strnlen.
 *
 * ALSO SEE
 *    safe_strnterminate()
 *
 */
rsize_t
safe_strnlen(const char *dest, rsize_t dmax)
{
    rsize_t count;

    if (dest == NULL) {
        return RCNEGATE(0);
    }

    if (dmax == 0) {
        invoke_safe_str_constraint_handler("safe_strnlen: dmax is 0",
                   NULL, ESZEROL);
        return RCNEGATE(0);
    }

    if (dmax > RSIZE_MAX_STR) {
        invoke_safe_str_constraint_handler("safe_strnlen: dmax exceeds max",
                   NULL, ESLEMAX);
        return RCNEGATE(0);
    }

    count = 0;
    while (*dest && dmax) {
        count++;
        dmax--;
        dest++;
    }

    return RCNEGATE(count);
}
EXPORT_SYMBOL(safe_strnlen)
