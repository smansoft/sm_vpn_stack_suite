/*------------------------------------------------------------------
 * safe_safe_memset32
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
 *    safe_memset32 - Sets a block of memory to value
 *
 * SYNOPSIS
 *    #include "safe_mem_lib.h"
 *    errno_t
 *    safe_memset32(uint32_t *dest, rsize_t len, uint32_t value)
 *
 * DESCRIPTION
 *    Sets len uint32_t starting at dest to the specified value.
 *
 * EXTENSION TO
 *    ISO/IEC JTC1 SC22 WG14 N1172, Programming languages, environments
 *    and system software interfaces, Extensions to the C Library,
 *    Part I: Bounds-checking interfaces
 *
 * INPUT PARAMETERS
 *    dest       pointer to memory that will be set to the value
 *
 *    len        number of uint32_t to be set
 *
 *    value      uint32_t value to be written
 *
 * OUTPUT PARAMETERS
 *    dest      is updated
 *
 * RUNTIME CONSTRAINTS
 *    dest shall not be a null pointer.
 *    len shall not be 0 nor greater than RSIZE_MAX_MEM32.
 *    If there is a runtime constraint, the operation is not performed.
 *
 * RETURN VALUE
 *    EOK        successful operation
 *    ESNULLP    NULL pointer
 *    ESZEROL    zero length
 *    ESLEMAX    length exceeds max limit
 *
 * ALSO SEE
 *    safe_memset(), safe_memset16()
 *
 */
errno_t
safe_memset32 (uint32_t *dest, rsize_t len, uint32_t value)
{
    if (dest == NULL) {
        invoke_safe_mem_constraint_handler("safe_memset32: dest is null",
                   NULL, ESNULLP);
        return (RCNEGATE(ESNULLP));
    }

    if (len == 0) {
        invoke_safe_mem_constraint_handler("safe_memset32: len is 0",
                   NULL, ESZEROL);
        return (RCNEGATE(ESZEROL));
    }

    if (len > RSIZE_MAX_MEM32) {
        invoke_safe_mem_constraint_handler("safe_memset32: len  exceeds max",
                   NULL, ESLEMAX);
        return (RCNEGATE(ESLEMAX));
    }

    safe_mem_prim_set32(dest, len, value);

    return (RCNEGATE(EOK));
}
EXPORT_SYMBOL(safe_memset32)
