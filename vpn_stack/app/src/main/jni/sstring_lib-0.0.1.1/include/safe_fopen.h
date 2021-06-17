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
	type for definition: shared or non-shared opening of the file
*/
typedef enum _sm_safe_fopen_shared_type
{
	SM_SFOPEN_NSHARED_TYPE = 0,
	SM_SFOPEN_SHARED_TYPE = 1
} sm_safe_fopen_shared_type;

/*
	multiplaform version of secure file opening of file with shared opening support;
	args:
		file	- result stream file object, that will be opened 
		fpath	- file path
		mode	- mode, fow example: "r","w","w+","a"
		shared	- shared object (shared or non-shared opening of the file)  
*/
errno_t safe_fopen(FILE** file, const char* const fpath, const char* mode, const sm_safe_fopen_shared_type shared);

#ifdef __cplusplus
}
#endif
