#ifdef ANDROID
#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))
#else
#include <cstdio>

#define LOGI(...) printf(__VA_ARGS__);
#define LOGW(...) printf(__VA_ARGS__);
#endif

