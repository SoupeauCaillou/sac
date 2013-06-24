#pragma once

extern bool AssertOnFatal;

//to handle vec2 operator<<
#include <ostream>
#include <glm/glm.hpp>
inline std::ostream& operator<<(std::ostream& stream, const glm::vec2 & v) {
    return stream << v.x << ", " << v.y;
}

#if SAC_ENABLE_LOG

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
std::ostream& logToStream(std::ostream& stream, LogVerbosity::Enum type, const char* file, int line);
std::ostream& vlogToStream(std::ostream& stream, int level, const char* file, int line);



#include <cassert>

#if SAC_ANDROID
#include <sstream>
#include <android/log.h>
#define SAC_LOG_PRE std::stringstream __log_ss;
#define SAC_LOG_STREAM __log_ss
#define SAC_LOG_POST __android_log_print(ANDROID_LOG_INFO, "sac", "%s", __log_ss.str().c_str());
#else
#include <iostream>
#define SAC_LOG_PRE
#define SAC_LOG_STREAM std::cout
#define SAC_LOG_POST
#endif

#define __LOG(level, x) \
    if ((int)logLevel >= (int)level) {\
        SAC_LOG_PRE \
        logToStream(SAC_LOG_STREAM, level, __FILE__, __LINE__) << x << std::endl; \
        SAC_LOG_POST \
        \
        if (level == LogVerbosity::FATAL && AssertOnFatal) { \
            assert(#x && 0); \
        } \
    }

#define __LOG_WHILE(level, x) do { \
    __LOG(level, x) \
} while (false)

#define __LOG_IF_WHILE(cond, level, x) do { \
    if ((cond)) \
        __LOG(level, x) \
} while (false)

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
