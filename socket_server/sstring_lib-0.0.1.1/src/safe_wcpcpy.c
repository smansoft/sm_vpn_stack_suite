/*------------------------------------------------------------------
 * safe_wcpcpy.c
 *
 * August 2014, D Wheeler
 *
 * Copyright (c) 2014 by Intel Corp
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
 *    safe_wcpcpy
 *
 * SYNOPSIS
 *    #include "safe_str_lib.h"
 *    wchar_t *
 *    safe_wcpcpy(wchar_t* dest, rsize_t dmax, const wchar_t* src, errno_t *err)
 *
 * DESCRIPTION
 *    The safe_wcpcpy function copies the wide character string pointed
 *    to by src (including the terminating null character) into the
 *    array pointed to by dest, and returns a pointer to the end of
 *    the wide character string. All elements following the terminating
 *    null character (if any) written by safe_wcpcpy in the array of
 *    dmax characters pointed to by dest are nulled when
 *    safe_wcpcpy returns.
 *
 * SPECIFIED IN
 *    ISO/IEC TR 24731, Programming languages, environments
 *    and system software interfaces, Extensions to the C Library,
 *    Part I: Bounds-checking interfaces
 *
 * INPUT PARAMETERS
 *    dest      pointer to string that will be replaced by src.
 *
 *    dmax      restricted maximum length of dest
 *
 *    src       pointer to the wide character string that will be copied
 *              to dest
 *
 *    err      the error code upon error, or EOK if successful
 *
 * OUTPUT PARAMETERS
 *    dest      updated
 *    err       updated as follows:
 *    			  EOK        successful operation, the characters in src were
 *               		     copied into dest and the result is null terminated.
 *    			  ESNULLP    NULL pointer
 *    			  ESZEROL    zero length
 *    			  ESLEMAX    length exceeds max limit
 *    			  ESOVRLP    strings overlap
 *    			  ESNOSPC    not enough space to copy src
 *
 * RUNTIME CONSTRAINTS
 *    Neither dest nor src shall be a null pointer.
 *    dmax shall not be greater than RSIZE_MAX_STR.
 *    dmax shall not equal zero.
 *    dmax shall be greater than safe_strnlen(src, dmax).
 *    Copying shall not take place between objects that overlap.
 *    If there is a runtime-constraint violation, then if dest
 *       is not a null pointer and destmax is greater than zero and
 *       not greater than RSIZE_MAX_STR, then safe_strcpy nulls dest.
 *
 * RETURN VALUE
 *   a wchar_t pointer to the terminating null at the end of dest
 *
 * ALSO SEE
 *    safe_wcscpy(), safe_wcscat(), safe_wcsncat(), safe_wcsncpy()
 *    safe_strcpy, safe_strcat(), safe_strncat(), safe_strncpy()
 *
 */
wchar_t *
safe_wcpcpy(wchar_t* dest, rsize_t dmax, const wchar_t* src, errno_t *err)
{
    rsize_t orig_dmax;
    wchar_t *orig_dest;
    const wchar_t *overlap_bumper;

    if (dest == NULL) {
        invoke_safe_str_constraint_handler("safe_wcpcpy: dest is null",
                   NULL, ESNULLP);
        *err = RCNEGATE(ESNULLP);
        return NULL;
    }

    if (dmax == 0) {
        invoke_safe_str_constraint_handler("safe_wcpcpy: dmax is 0",
                   NULL, ESZEROL);
        *err = RCNEGATE(ESZEROL);
        return NULL;
    }

    if (dmax*sizeof(wchar_t) > RSIZE_MAX_STR) {
        invoke_safe_str_constraint_handler("safe_wcpcpy: dmax exceeds max",
                   NULL, ESLEMAX);
        *err = RCNEGATE(ESLEMAX);
        return NULL;
    }

    if (src == NULL) {
#ifdef SAFECLIB_STR_NULL_SLACK
        /* null string to clear data */
        while (dmax) {  *dest = L'\0'; dmax--; dest++; }
#else
        *dest = L'\0';
#endif
        invoke_safe_str_constraint_handler("safe_wcpcpy: src is null",
                   NULL, ESNULLP);
        *err = RCNEGATE(ESNULLP);
        return NULL;
    }

    if (dest == src) {
    	/* move dest to the end of the string */
    	while (dmax && (*dest != L'\0')) { dmax--; dest++; }
    	if ( *dest != L'\0' ) {
    		invoke_safe_str_constraint_handler("safe_wcpcpy: no null terminator in dest",
    		                   NULL, ESLEMAX);
			*err = RCNEGATE(ESLEMAX);
			return NULL;
    	}
    	*err = RCNEGATE(EOK);
    	return dest;
    }

    /* hold base of dest in case src was not copied */
    orig_dmax = dmax;
    orig_dest = dest;

    if (dest < src) {
        overlap_bumper = src;

        while (dmax > 0) {
            if (dest == overlap_bumper) {
                safe_handle_wc_error(orig_dest, orig_dmax, "safe_wcpcpy: overlapping objects",
                             ESOVRLP);
    			*err = RCNEGATE(ESOVRLP);
    			return NULL;
            }

            *dest = *src;
            if (*dest == L'\0') {
#ifdef SAFECLIB_STR_NULL_SLACK
                /* null slack to clear any data */
                while (dmax) { *dest = L'\0'; dmax--; dest++; }
#endif
                *err = RCNEGATE(EOK);
                return dest; /* successful return */
            }

            dmax--;
            dest++;
            src++;
        }

    } else {
        overlap_bumper = dest;

        while (dmax > 0) {
            if (src == overlap_bumper) {
                safe_handle_wc_error(orig_dest, orig_dmax, "safe_wcpcpy: overlapping objects",
                      ESOVRLP);
    			*err = RCNEGATE(ESOVRLP);
    			return NULL;
            }

            *dest = *src;
            if (*dest == L'\0') {
#ifdef SAFECLIB_STR_NULL_SLACK
                /* null slack to clear any data */
                while (dmax) { *dest = L'\0'; dmax--; dest++; }
#endif
                *err = RCNEGATE(EOK);
                return dest; /* successful return */
            }

            dmax--;
            dest++;
            src++;
        }
    }

    /*
     * the entire src must have been copied, if not reset dest
     * to null the string.
     */
    safe_handle_wc_error(orig_dest, orig_dmax, "safe_wcpcpy: not enough space for src",
                 ESNOSPC);
    *err = RCNEGATE(ESNOSPC);
    return NULL;
}
EXPORT_SYMBOL(safe_wcpcpy)
