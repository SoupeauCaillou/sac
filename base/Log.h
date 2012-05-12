#pragma once

#include <stdint.h>

#ifdef ANDROID
#include <android/log.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "tilematchC", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "tilematchC", __VA_ARGS__))
#else
#include <cstdio>
extern bool __log_enabled;

#define LOGI(...) (__log_enabled & printf("[%s] ", __PRETTY_FUNCTION__) & printf(__VA_ARGS__) & printf("\n"))
#define LOGW(...) (__log_enabled & printf("[%s] ", __PRETTY_FUNCTION__) & printf(__VA_ARGS__) & printf("\n"))
#endif
