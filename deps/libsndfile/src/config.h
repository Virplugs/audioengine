#pragma once

#define CPU_IS_BIG_ENDIAN 0
#define CPU_IS_LITTLE_ENDIAN 1

#define PACKAGE_NAME "sndfile"
#define PACKAGE_VERSION "0"

#define CPU_CLIPS_NEGATIVE 0
#define CPU_CLIPS_POSITIVE 0

#define SIZEOF_SF_COUNT_T 8

#ifdef _WIN32
#define OS_IS_WIN32 1
#define USE_WINDOWS_API 1
#define HAVE_FTRUNCATE 0
#define HAVE_UNISTD_H 0

#include <io.h>
#define access _access

#else
#define OS_IS_WIN32 0
#define USE_WINDOWS_API 0
#define HAVE_FTRUNCATE 1
#define HAVE_UNISTD_H 1
#endif

#define HAVE_GMTIME 1

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288
#endif
