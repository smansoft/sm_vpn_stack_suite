/*------------------------------------------------------------------
 * safe_memmove.c
 *
 * October 2008, Bo Berry
 *
 * Copyright (c) 2008-2011 Cisco Systems
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
#include "safe_mem_constraint.h"
#include "safe_mem_primitives_lib.h"
#include "safe_mem_lib.h"


/**
 * NAME
 *    safe_memmove
 *
 * SYNOPSIS
 *    #include "safe_mem_lib.h"
 *    errno_t
 *    safe_memmove(void *dest, rsize_t dmax,
 *              const void *src, rsize_t smax)
 *
 * DESCRIPTION
 *    The safe_memmove function copies smax bytes from the region pointed
 *    to by src into the region pointed to by dest. This copying takes place
 *    as if the smax bytes from the region pointed to by src are first copied
 *    into a temporary array of smax bytes that does not overlap the region
 *    pointed to by dest or src, and then the smax bytes from the temporary
 *    array are copied into the object region to by dest.
 *
 * SPECIFIED IN
 *    ISO/IEC TR 24731, Programming languages, environments
 *    and system software interfaces, Extensions to the C Library,
 *    Part I: Bounds-checking interfaces
 *
 * INPUT PARAMETERS
 *    dest       pointer to the memory that will be replaced by src.
 *
 *    dmax       maximum length of the resulting dest, in bytes
 *
 *    src        pointer to the memory that will be copied
 *                to dest
 *
 *    smax       maximum number bytes of src that can be copied
 *
 *  OUTPUT PARAMETERS
 *    dest      is updated
 *
 * RUNTIME CONSTRAINTS
 *    Neither dest nor src shall be a null pointer.
 *    Neither dmax nor smax shall be 0.
 *    dmax shall not be greater than RSIZE_MAX_MEM.
 *    smax shall not be greater than dmax.
 *    If there is a runtime-constraint violation, the safe_memmove function
 *    stores zeros in the first dmax characters of the regionpointed to
 *    by dest if dest is not a null pointer and dmax is not greater
 *    than RSIZE_MAX_MEM.
 *
 * RETURN VALUE
 *    EOK        successful operation
 *    ESNULLP    NULL pointer
 *    ESZEROL    zero length
 *    ESLEMAX    length exceeds max limit
 *
 * ALSO SEE
 *  safe_memmove16(), safe_memmove32(), safe_memcpy(), safe_memcpy16(), safe_memcpy32()
 *
 */
errno_t
safe_memmove(void *dest, rsize_t dmax, const void *src, rsize_t smax)
{
    uint8_t *dp;
    const uint8_t  *sp;

    dp= dest;
    sp = src;

    if (dp == NULL) {
        invoke_safe_mem_constraint_handler("safe_memmove: dest is null",
                   NULL, ESNULLP);
        return (RCNEGATE(ESNULLP));
    }

    if (dmax == 0) {
        invoke_safe_mem_constraint_handler("safe_memmove: dmax is 0",
                   NULL, ESZEROL);
        return (RCNEGATE(ESZEROL));
    }

    if (dmax > RSIZE_MAX_MEM) {
        invoke_safe_mem_constraint_handler("safe_memmove: dmax exceeds max",
                   NULL, ESLEMAX);
        return (RCNEGATE(ESLEMAX));
    }

    if (smax == 0) {
        safe_mem_prim_set(dp, dmax, 0);
        invoke_safe_mem_constraint_handler("safe_memmove: smax is 0",
                   NULL, ESZEROL);
        return (RCNEGATE(ESZEROL));
    }

    if (smax > dmax) {
        safe_mem_prim_set(dp, dmax, 0);
        invoke_safe_mem_constraint_handler("safe_memmove: smax exceeds max",
                   NULL, ESLEMAX);
        return (RCNEGATE(ESLEMAX));
    }

    if (sp == NULL) {
        safe_mem_prim_set(dp, dmax, 0);
        invoke_safe_mem_constraint_handler("safe_memmove: src is null",
                   NULL, ESNULLP);
        return (RCNEGATE(ESNULLP));
    }

    /*
     * now perform the copy
     */
    safe_mem_prim_move(dp, sp, smax);

    return (RCNEGATE(EOK));
}
EXPORT_SYMBOL(safe_memmove)
