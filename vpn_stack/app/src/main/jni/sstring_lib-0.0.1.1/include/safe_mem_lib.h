/*------------------------------------------------------------------
 * safe_mem_lib.h -- Safe C Library Memory APIs
 *
 * October 2008, Bo Berry
 * Modified 2012, Jonathan Toppins <jtoppins@users.sourceforge.net>
 *
 * Copyright (c) 2008-2012 by Cisco Systems, Inc.
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

#ifndef __SAFE_MEM_LIB_H__
#define __SAFE_MEM_LIB_H__

#include "safe_lib.h"
#include <wchar.h>

#define RSIZE_MAX_MEM      ( 256UL << 20 )     /* 256MB */
#define RSIZE_MAX_MEM16    ( RSIZE_MAX_MEM/2 )
#define RSIZE_MAX_MEM32    ( RSIZE_MAX_MEM/4 )

#if defined __cplusplus
extern "C" {
#endif

/* set memory constraint handler */
extern safe_constraint_handler_t
set_safe_mem_constraint_handler(safe_constraint_handler_t handler);


/* compare memory */
extern errno_t safe_memcmp(const void *dest, rsize_t dmax,
                        const void *src, rsize_t slen, int *diff);

/* compare uint16_t memory */
extern errno_t safe_memcmp16(const uint16_t *dest, rsize_t dmax,
                          const uint16_t *src, rsize_t slen, int *diff);

/* compare uint32_t memory */
extern errno_t safe_memcmp32(const uint32_t *dest, rsize_t dmax,
                          const uint32_t *src, rsize_t slen, int *diff);

/* wide compare memory */
extern errno_t safe_wmemcmp(const wchar_t *dest, rsize_t dmax,
                         const wchar_t *src,  rsize_t smax, int *diff);


/* copy memory */
extern errno_t safe_memcpy(void *dest, rsize_t dmax,
                        const void *src, rsize_t slen);

/* copy uint16_t memory */
extern errno_t safe_memcpy16(uint16_t *dest, rsize_t dmax,
                          const uint16_t *src, rsize_t slen);

/* copy uint32_t memory */
extern errno_t safe_memcpy32(uint32_t *dest, rsize_t dmax,
                          const uint32_t *src, rsize_t slen);

/* copy wchar_t memory */
extern errno_t safe_wmemcpy(wchar_t *dest, rsize_t dmax,
                          const wchar_t *src, rsize_t slen);


/* move memory, including overlapping memory */
extern errno_t safe_memmove(void *dest, rsize_t  dmax,
                          const void *src, rsize_t slen);

/* uint16_t move memory, including overlapping memory */
extern errno_t safe_memmove16(uint16_t *dest, rsize_t dmax,
                            const uint16_t *src, rsize_t slen);

/* uint32_t move memory, including overlapping memory */
extern errno_t safe_memmove32(uint32_t *dest, rsize_t dmax,
                            const uint32_t *src, rsize_t slen);

/* copy wchar_t memory, including overlapping memory */
extern errno_t safe_wmemmove(wchar_t *dest, rsize_t dmax,
                          const wchar_t *src, rsize_t slen);

/* set bytes */
extern errno_t safe_memset(void *dest, rsize_t dmax, uint8_t value);

/* set uint16_t */
extern errno_t safe_memset16(uint16_t *dest, rsize_t dmax, uint16_t value);

/* set uint32_t */
extern errno_t safe_memset32(uint32_t *dest, rsize_t dmax, uint32_t value);

/* wide set bytes */
extern errno_t safe_wmemset(wchar_t *dest, wchar_t value, rsize_t len);


/* byte zero */
extern errno_t safe_memzero(void *dest, rsize_t dmax);

/* uint16_t zero */
extern errno_t safe_memzero16(uint16_t *dest, rsize_t dmax);

/* uint32_t zero */
extern errno_t safe_memzero32(uint32_t *dest, rsize_t dmax);

#if defined __cplusplus
}
#endif

#endif  /* __SAFE_MEM_LIB_H__ */
