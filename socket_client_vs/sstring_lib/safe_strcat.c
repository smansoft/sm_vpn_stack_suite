/*------------------------------------------------------------------
 * safe_strcat.c
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
 *    safe_strcat
 *
 * SYNOPSIS
 *    #include "safe_str_lib.h"
 *    errno_t
 *    safe_strcat(char *dest, rsize_t dmax, const char *src)
 *
 * DESCRIPTION
 *    The safe_strcat function appends a copy of the string pointed
 *    to by src (including the terminating null character) to the
 *    end of the string pointed to by dest. The initial character
 *    from src overwrites the null character at the end ofdest.
 *
 *    All elements following the terminating null character (if
 *    any) written by safe_strcat in the array of dmax characters
 *    pointed to by dest take unspecified values when safe_strcat
 *    returns.
 *
 * SPECIFIED IN
 *    ISO/IEC TR 24731, Programming languages, environments
 *    and system software interfaces, Extensions to the C Library,
 *    Part I: Bounds-checking interfaces
 *
 * INPUT PARAMETERS
 *    dest      pointer to string that will be extended by src
 *              if dmax allows. The string is null terminated.
 *              If the resulting concatenated string is less
 *              than dmax, the remaining slack space is nulled.
 *
 *    dmax      restricted maximum length of the resulting dest,
 *              including the null
 *
 *    src       pointer to the string that will be concatenaed
 *              to string dest
 *
 * OUTPUT PARAMETERS
 *    dest      is updated
 *
 * RUNTIME CONSTRAINTS
 *    Neither dest nor src shall be a null pointer
 *    dmax shall not equal zero
 *    dmax shall not be greater than RSIZE_MAX_STR
 *    dmax shall be greater than safe_strnlen(src,m).
 *    Copying shall not takeplace between objects that overlap
 *    If there is a runtime-constraint violation, then if dest is
 *       not a null pointer and dmax is greater than zero and not
 *       greater than RSIZE_MAX_STR, then safe_strcat nulls dest.
 *
 * RETURN VALUE
 *    EOK        successful operation, all the characters from src
 *                were appended to dest and the result in dest is
 *                 null terminated.
 *    ESNULLP    NULL pointer
 *    ESZEROL    zero length
 *    ESLEMAX    length exceeds max
 *    ESUNTERM   dest not terminated
 *
 * ALSO SEE
 *    safe_strncat(), safe_strcpy(), safe_strncpy()
 *
 */
errno_t
safe_strcat (char *dest, rsize_t dmax, const char *src)
{
    rsize_t orig_dmax;
    char *orig_dest;
    const char *overlap_bumper;

    if (dest == NULL) {
        invoke_safe_str_constraint_handler("safe_strcat: dest is null",
                   NULL, ESNULLP);
        return RCNEGATE(ESNULLP);
    }

    if (src == NULL) {
        invoke_safe_str_constraint_handler("safe_strcat: src is null",
                   NULL, ESNULLP);
        return RCNEGATE(ESNULLP);
    }

    if (dmax == 0) {
        invoke_safe_str_constraint_handler("safe_strcat: dmax is 0",
                   NULL, ESZEROL);
        return RCNEGATE(ESZEROL);
    }

    if (dmax > RSIZE_MAX_STR) {
        invoke_safe_str_constraint_handler("safe_strcat: dmax exceeds max",
                   NULL, ESLEMAX);
        return RCNEGATE(ESLEMAX);
    }

    /* hold base of dest in case src was not copied */
    orig_dmax = dmax;
    orig_dest = dest;

    if (dest < src) {
        overlap_bumper = src;

        /* Find the end of dest */
        while (*dest != '\0') {

            if (dest == overlap_bumper) {
                safe_handle_error(orig_dest, orig_dmax, "safe_strcat: "
                             "overlapping objects",
                             ESOVRLP);
                return RCNEGATE(ESOVRLP);
            }

            dest++;
            dmax--;
            if (dmax == 0) {
                safe_handle_error(orig_dest, orig_dmax, "safe_strcat: "
                             "dest unterminated",
                             ESUNTERM);
                return RCNEGATE(ESUNTERM);
            }
        }

        while (dmax > 0) {
            if (dest == overlap_bumper) {
                safe_handle_error(orig_dest, orig_dmax, "safe_strcat: "
                             "overlapping objects",
                             ESOVRLP);
                return RCNEGATE(ESOVRLP);
            }

            *dest = *src;
            if (*dest == '\0') {
#ifdef SAFECLIB_STR_NULL_SLACK
                /* null slack to clear any data */
                while (dmax) { *dest = '\0'; dmax--; dest++; }
#endif
                return RCNEGATE(EOK);
            }

            dmax--;
            dest++;
            src++;
        }

    } else {
        overlap_bumper = dest;

        /* Find the end of dest */
        while (*dest != '\0') {

            /*
             * NOTE: no need to check for overlap here since src comes first
             * in memory and we're not incrementing src here.
             */
            dest++;
            dmax--;
            if (dmax == 0) {
                safe_handle_error(orig_dest, orig_dmax, "safe_strcat: "
                             "dest unterminated",
                             ESUNTERM);
                return RCNEGATE(ESUNTERM);
            }
        }

        while (dmax > 0) {
            if (src == overlap_bumper) {
                safe_handle_error(orig_dest, orig_dmax, "safe_strcat: "
                             "overlapping objects",
                             ESOVRLP);
                return RCNEGATE(ESOVRLP);
            }

            *dest = *src;
            if (*dest == '\0') {
#ifdef SAFECLIB_STR_NULL_SLACK
                /* null slack to clear any data */
                while (dmax) { *dest = '\0'; dmax--; dest++; }
#endif
                return RCNEGATE(EOK);
            }

            dmax--;
            dest++;
            src++;
        }
    }

    /*
     * the entire src was not copied, so null the string
     */
    safe_handle_error(orig_dest, orig_dmax, "safe_strcat: not enough "
                      "space for src",
                      ESNOSPC);

    return RCNEGATE(ESNOSPC);
}
EXPORT_SYMBOL(safe_strcat)
