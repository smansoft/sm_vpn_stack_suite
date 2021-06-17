/*
 *    Copyright (c) 2020-2021 SManSoft <http://www.smansoft.com/>
 *    Sergey Manoylo <info@smansoft.com>
 */

#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include <math.h>
#include <ctype.h>

#if defined SM_OS_WINDOWS
#include <windows.h>
#include <process.h>
#include <share.h>
#endif

#if defined SM_OS_LINUX
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#if defined SM_SYNC_LOG
#include <pthread.h>
#endif
#endif

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
