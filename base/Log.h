/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <stdint.h>

#ifdef ANDROID
#include <android/log.h>
#ifdef ENABLE_LOG
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "sacC", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "sacC", __VA_ARGS__))
#else
#define LOGI(...)
#define LOGW(...)
#endif
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "sacC", __VA_ARGS__))

#else
#include <cstdio>
extern bool __log_enabled;

#define LOGE(...) if (__log_enabled) {printf("ERRO-[%s] ", __PRETTY_FUNCTION__); printf(__VA_ARGS__); printf("\n");}
#define LOGW(...) if (__log_enabled) {printf("WARN-[%s] ", __PRETTY_FUNCTION__);printf(__VA_ARGS__);printf("\n");}
#define LOGI(...) if (__log_enabled) {printf("INFO-[%s] ", __PRETTY_FUNCTION__);printf(__VA_ARGS__);printf("\n");}

#endif
