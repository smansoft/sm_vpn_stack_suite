/*------------------------------------------------------------------
 * safe_str_lib.h -- Safe C Library String APIs
 *
 * October 2008, Bo Berry
 *
 * Copyright (c) 2008-2011, 2013 by Cisco Systems, Inc.
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

#ifndef __SAFE_STR_LIB_H__
#define __SAFE_STR_LIB_H__

#include <wchar.h>

#include "safe_lib.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * The shortest string is a null string!!
 */
#define RSIZE_MIN_STR      ( 1 )

/* maximum sring length */
#define RSIZE_MAX_STR      ( 4UL << 10 )      /* 4KB */


/* The makeup of a password */
#define SAFE_STR_MIN_LOWERCASE     ( 2 )
#define SAFE_STR_MIN_UPPERCASE     ( 2 )
#define SAFE_STR_MIN_NUMBERS       ( 1 )
#define SAFE_STR_MIN_SPECIALS      ( 1 )

#define SAFE_STR_PASSWORD_MIN_LENGTH   ( 6 )
#define SAFE_STR_PASSWORD_MAX_LENGTH   ( 32 )


/* set string constraint handler */
extern safe_constraint_handler_t
set_str_constraint_handler_s(safe_constraint_handler_t handler);


/* string compare */
extern errno_t
safe_strcasecmp(const char *dest, rsize_t dmax,
             const char *src, int *indicator);


/* find a substring _ case insensitive */
extern errno_t
safe_strcasestr(char *dest, rsize_t dmax,
             const char *src, rsize_t slen, char **substring);


/* string concatenate */
extern errno_t
safe_strcat(char *dest, rsize_t dmax, const char *src);


/* string compare */
extern errno_t
safe_strcmp(const char *dest, rsize_t dmax,
         const char *src, int *indicator);


/* fixed field string compare */
extern errno_t
safe_strcmpfld(const char *dest, rsize_t dmax,
            const char *src, int *indicator);


/* string copy */
extern errno_t
safe_strcpy(char *dest, rsize_t dmax, const char *src);

/* string copy */
extern char *
safe_stpcpy(char *dest, rsize_t dmax, const char *src, errno_t *err);

/* string copy */
extern char *
safe_stpncpy(char *dest, rsize_t dmax, const char *src, rsize_t smax, errno_t *err);

/* fixed char array copy */
extern errno_t
safe_strcpyfld(char *dest, rsize_t dmax, const char *src, rsize_t slen);


/* copy from a null terminated string to fixed char array */
extern errno_t
safe_strcpyfldin(char *dest, rsize_t dmax, const char *src, rsize_t slen);


/* copy from a char array to null terminated string */
extern errno_t
safe_strcpyfldout(char *dest, rsize_t dmax, const char *src, rsize_t slen);


/* computes excluded prefix length */
extern errno_t
safe_strcspn(const char *dest, rsize_t dmax,
          const char *src,  rsize_t slen, rsize_t *count);


/* returns a pointer to the first occurrence of c in dest */
extern errno_t
safe_strfirstchar(char *dest, rsize_t dmax, char c, char **first);


/* returns index of first difference */
extern  errno_t
safe_strfirstdiff(const char *dest, rsize_t dmax,
               const char *src, rsize_t *index);


/* validate alphanumeric string */
extern bool
safe_strisalphanumeric(const char *str, rsize_t slen);


/* validate ascii string */
extern bool
safe_strisascii(const char *str, rsize_t slen);


/* validate string of digits */
extern bool
safe_strisdigit(const char *str, rsize_t slen);


/* validate hex string */
extern bool
safe_strishex(const char *str, rsize_t slen);


/* validate lower case */
extern bool
safe_strislowercase(const char *str, rsize_t slen);


/* validate mixed case */
extern bool
safe_strismixedcase(const char *str, rsize_t slen);


/* validate password */
extern bool
safe_strispassword(const char *str, rsize_t slen);


/* validate upper case */
extern bool
safe_strisuppercase(const char *str, rsize_t slen);


/* returns  a pointer to the last occurrence of c in s1 */
extern errno_t
safe_strlastchar(char *str, rsize_t smax, char c, char **first);


/* returns index of last difference */
extern  errno_t
safe_strlastdiff(const char *dest, rsize_t dmax,
              const char *src, rsize_t *index);


/* left justify */
extern errno_t
safe_strljustify(char *dest, rsize_t dmax);


/* fitted string concatenate */
extern errno_t
safe_strncat(char *dest, rsize_t dmax, const char *src, rsize_t slen);


/* fitted string copy */
extern errno_t
safe_strncpy(char *dest, rsize_t dmax, const char *src, rsize_t slen);


/* string length */
extern rsize_t
safe_strnlen(const char *s, rsize_t smax);


/* string terminate */
extern rsize_t
safe_strnterminate(char *s, rsize_t smax);


/* get pointer to first occurrence from set of char */
extern errno_t
safe_strpbrk(char *dest, rsize_t dmax,
          char *src,  rsize_t slen, char **first);


extern errno_t
safe_strfirstsame(const char *dest, rsize_t dmax,
               const char *src,  rsize_t *index);

extern errno_t
safe_strlastsame(const char *dest, rsize_t dmax,
              const char *src, rsize_t *index);


/* searches for a prefix */
extern errno_t
safe_strprefix(const char *dest, rsize_t dmax, const char *src);


/* removes leading and trailing white space */
extern errno_t
safe_strremovews(char *dest, rsize_t dmax);


/* computes inclusive prefix length */
extern errno_t
safe_strspn(const char *dest, rsize_t dmax,
         const char *src,  rsize_t slen, rsize_t *count);


/* find a substring */
extern errno_t
safe_strstr(char *dest, rsize_t dmax,
         const char *src, rsize_t slen, char **substring);


/* string tokenizer */
extern char *
safe_strtok(char *s1, rsize_t *s1max, const char *src, char **ptr);


/* convert string to lowercase */
extern errno_t
safe_strtolowercase(char *str, rsize_t slen);


/* convert string to uppercase */
extern errno_t
safe_strtouppercase(char *str, rsize_t slen);


/* zero an entire string with nulls */
extern errno_t
safe_strzero(char *dest, rsize_t dmax);


/* wide string copy */
extern wchar_t *
safe_wcpcpy(wchar_t* dest, rsize_t dmax, const wchar_t* src, errno_t *err);

/* wide string concatenate */
extern errno_t
safe_wcscat(wchar_t* dest, rsize_t dmax, const wchar_t* src);

/* fitted wide string concatenate */
extern errno_t
safe_wcsncat(wchar_t *dest, rsize_t dmax, const wchar_t *src, rsize_t slen);

/* wide string copy */
errno_t
safe_wcscpy(wchar_t* dest, rsize_t dmax, const wchar_t* src);

/* fitted wide string copy */
extern errno_t
safe_wcsncpy(wchar_t* dest, rsize_t dmax, const wchar_t* src, rsize_t slen);

/* wide string length */
extern rsize_t
safe_wcsnlen(const wchar_t *dest, rsize_t dmax);

#ifdef __cplusplus
}
#endif

#endif   /* __SAFE_STR_LIB_H__ */
