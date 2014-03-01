#pragma once

#if SAC_WINDOWS
typedef unsigned char uint8_t;
typedef unsigned __int32 uint32_t;
#endif

#if SAC_WINDOWS
#define PRAGMA_WARNING(x) __pragma(x)
#else
#define PRAGMA_WARNING(x) _Pragma(#x)
#endif
