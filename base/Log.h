/*
    This file is part of Soupe Au Caillou.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

    Soupe Au Caillou is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Soupe Au Caillou is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Soupe Au Caillou.  If not, see <http://www.gnu.org/licenses/>.
*/



#pragma once

extern bool AssertOnFatal;

//to handle vec2 operator<<
#include <ostream>
#include <iomanip>

#include <glm/glm.hpp>
inline std::ostream& operator<<(std::ostream& stream, const glm::vec2 & v) {
    return stream << v.x << ", " << v.y;
}

#undef ERROR
#include <map>



namespace LogVerbosity {
	enum Enum {
		FATAL = 0,
		ERROR,
        TODO,
		WARNING,
		INFO,
        VERBOSE1,
        VERBOSE2
	};
}

extern LogVerbosity::Enum logLevel;
extern std::map<std::string, bool> verboseFilenameFilters;
int logHeaderLength(const char* file, int line);
std::ostream& logToStream(std::ostream& stream, LogVerbosity::Enum type, const char* file, int line);
std::ostream& vlogToStream(std::ostream& stream, int level, const char* file, int line);



#include <signal.h>

#if SAC_ANDROID

#include <android/log.h>
static const android_LogPriority level2prio[] {
    ANDROID_LOG_FATAL,
    ANDROID_LOG_ERROR,
    ANDROID_LOG_WARN,
    ANDROID_LOG_WARN,
    ANDROID_LOG_INFO,
    ANDROID_LOG_VERBOSE,
    ANDROID_LOG_VERBOSE
};
#include <sstream>
#define SAC_LOG_PRE std::stringstream __log_ss;
#define SAC_LOG_STREAM __log_ss
#define SAC_LOG_POST __android_log_print(level2prio[logLevel], "sac", "%s", __log_ss.str().c_str());
#else
#include <iostream>
#define SAC_LOG_PRE
#define SAC_LOG_STREAM std::cout
#define SAC_LOG_POST
#endif

#if SAC_WINDOWS
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

#define __LOG(level, x) \
    if ((int)logLevel >= (int)level) {\
        SAC_LOG_PRE \
        logToStream(SAC_LOG_STREAM, level, __FILE__, __LINE__) << x << std::endl; \
        SAC_LOG_POST \
        \
        if (level == LogVerbosity::FATAL && AssertOnFatal) { \
            raise(SIGABRT); \
        } \
    }

#define __LOG_WHILE(level, x) do { \
    __LOG(level, x) \
} while (false)

#define __LOG_IF_WHILE(cond, level, x) do { \
    if ((cond)) \
        __LOG(level, x) \
} while (false)


#if SAC_ENABLE_LOG
    #define LOG_USAGE_ONLY(x) x

    #define LOG_OFFSET() \
        logHeaderLength(__FILE__,__LINE__)

    #define LOGF(x) __LOG_WHILE(LogVerbosity::FATAL, x)
    #define LOGE(x) __LOG_WHILE(LogVerbosity::ERROR, x)
    #define LOGT(x) __LOG_WHILE(LogVerbosity::TODO, "<--TODO-->" << x)
    #define LOGW(x) __LOG_WHILE(LogVerbosity::WARNING, x)
    #define LOGI(x) __LOG_WHILE(LogVerbosity::INFO, x)
    #define LOGV(verbosity, x) do {\
        if ((int)logLevel >= ((int)(LogVerbosity::INFO) + verbosity)) {\
            SAC_LOG_PRE\
            vlogToStream(SAC_LOG_STREAM, verbosity, __FILE__, __LINE__) << x << std::endl;\
            SAC_LOG_POST\
        }\
    } while (false)

    #define LOGF_IF(cond, x) __LOG_IF_WHILE(cond, LogVerbosity::FATAL, x)
    #define LOGE_IF(cond, x) __LOG_IF_WHILE(cond, LogVerbosity::ERROR, x)
    #define LOGT_IF(cond, x) __LOG_IF_WHILE(cond, LogVerbosity::TODO, x)
    #define LOGW_IF(cond, x) __LOG_IF_WHILE(cond, LogVerbosity::WARNING, x)
    #define LOGI_IF(cond, x) __LOG_IF_WHILE(cond, LogVerbosity::INFO, x)
    #define LOGV_IF(verbosity, cond, x) do {\
        if ((cond) && (int)logLevel >= (int)LogVerbosity::INFO + verbosity) {\
            SAC_LOG_PRE \
            vlogToStream(SAC_LOG_STREAM, verbosity, __FILE__, __LINE__) << x << std::endl; \
            SAC_LOG_POST \
        } \
    } while (false)

    #define LOGE_EVERY_N(n, x) do {\
        static unsigned __log_count = 0;\
        if ((++__log_count % n) == 0) {\
            __LOG(LogVerbosity::ERROR, x) \
        } \
    } while (false)

    #define LOGW_EVERY_N(n, x) do {\
        static unsigned __log_count = 0;\
        if ((++__log_count % n) == 0) {\
            __LOG(LogVerbosity::WARNING, x) \
        } \
    } while (false)

    #define LOGI_EVERY_N(n, x) do {\
        static unsigned __log_count = 0;\
        if ((++__log_count % n) == 0) {\
            __LOG(LogVerbosity::INFO, x) \
        } \
    } while (false)

    #define LOGT_EVERY_N(n, x) do {\
        static unsigned __log_count = 0;\
        if ((++__log_count % n) == 0) {\
            __LOG(LogVerbosity::TODO, x) \
        } \
    } while (false)

#else
    #define LOG_USAGE_ONLY(x) 

    #define LOG_OFFSET() 0
    #define LOGF(x) do { assert(!AssertOnFatal); } while (false)
    #define LOGE(x) do {} while(false)
    #define LOGT(x) do {} while(false)
    #define LOGW(x) do {} while(false)
    #define LOGI(x) do {} while(false)
    #define LOGV(verbosity, x) do {} while(false)

    #define LOGF_IF(cond, x) do { if (cond) assert(!AssertOnFatal); } while (false)
    #define LOGE_IF(cond, x) do {} while(false)
    #define LOGT_IF(cond, x) do {} while(false)
    #define LOGW_IF(cond, x) do {} while(false)
    #define LOGI_IF(cond, x) do {} while(false)
    #define LOGV_IF(verbosity, cond, x) do {} while(false)

    #define LOGE_EVERY_N(n, x) do {} while(false)
    #define LOGW_EVERY_N(n, x) do {} while(false)
    #define LOGI_EVERY_N(n, x) do {} while(false)
    #define LOGT_EVERY_N(n, x) do {} while(false)

#endif
