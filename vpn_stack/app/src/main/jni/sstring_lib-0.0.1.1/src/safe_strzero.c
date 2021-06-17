/*------------------------------------------------------------------
 * safe_strzero.c
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
 *    safe_strzero
 *
 * SYNOPSIS
 *    #include "safe_str_lib.h"
 *    errno_t
 *    safe_strzero(char *dest, rsize_t dmax)
 *
 * DESCRIPTION
 *    Nulls dmax characters of dest.  This function can be used
 *    to clear strings that contained sensitive data.
 *
 * EXTENSION TO
 *    ISO/IEC TR 24731, Programming languages, environments
 *    and system software interfaces, Extensions to the C Library,
 *    Part I: Bounds-checking interfaces
 *
 * INPUT PARAMETERS
 *    dest     pointer to string that will be nulled.
 *
 *    dmax     restricted maximum length of dest
 *
 * OUTPUT PARAMETERS
 *    dest     updated string
 *
 * RETURN VALUE
 *    EOK        successful operation
 *    ESNULLP    NULL pointer
 *    ESZEROL    zero length
 *    ESLEMAX    length exceeds max limit
 *
 * ALSO SEE
 *    safe_strispassword()
 *
 */
errno_t
safe_strzero (char *dest, rsize_t dmax)
{
    if (dest == NULL) {
        invoke_safe_str_constraint_handler("safe_strzero: dest is null",
                   NULL, ESNULLP);
        return (ESNULLP);
    }

    if (dmax == 0) {
        invoke_safe_str_constraint_handler("safe_strzero: dmax is 0",
                   NULL, ESZEROL);
        return (ESZEROL);
    }

    if (dmax > RSIZE_MAX_STR) {
        invoke_safe_str_constraint_handler("safe_strzero: dmax exceeds max",
                   NULL, ESLEMAX);
        return (ESLEMAX);
    }

    /* null string to eliminate data */
    while (dmax) {
        *dest = '\0';
        dmax--;
        dest++;
    }

    return (EOK);
}
EXPORT_SYMBOL(safe_strzero)
