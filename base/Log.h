#include <stdint.h>

#ifdef ANDROID
#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "tilematchC", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "tilematchC", __VA_ARGS__))
#else
#include <cstdio>

#define LOGI(...) (printf(__VA_ARGS__) & printf("\n"))
#define LOGW(...) (printf(__VA_ARGS__) & printf("\n"))
#endif
