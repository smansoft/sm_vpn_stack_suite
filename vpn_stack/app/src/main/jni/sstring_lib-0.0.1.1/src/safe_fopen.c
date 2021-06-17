/*
 *    Copyright (c) 2020-2021 SManSoft <http://www.smansoft.com/>
 *    Sergey Manoylo <info@smansoft.com>
 */

#include "safe_fopen.h"

errno_t safe_fopen(FILE** file, const char* const fpath, const char* mode, const sm_safe_fopen_shared_type shared)
{
	errno_t res = SAFE_RES_ERROR;
#if defined SM_OS_WINDOWS
	if (shared == SM_SFOPEN_SHARED_TYPE) {
		*file = _fsopen(fpath, mode, _SH_DENYNO);
		res = (*file) ? SAFE_RES_OK : SAFE_RES_ERROR;
	}
	else if (shared == SM_SFOPEN_NSHARED_TYPE)  {
		res = fopen_s(file, fpath, mode);
		res = (!res && *fpath) ? SAFE_RES_OK : SAFE_RES_ERROR;
	}
#elif defined SM_OS_LINUX
	*file = fopen(fpath, mode);
	res = (*file != NULL) ? SAFE_RES_OK : SAFE_RES_ERROR;
#else
	#pragma message("Error: function safe_fopen is wrong, as SM_OS_WINDOWS and SM_OS_LINUX not defined")
#endif
	return res;
}
