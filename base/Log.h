#pragma once

#if SAC_ENABLE_LOG

#undef ERROR
#include <ostream>
#include <map>

//to handle vec2 operator<<
#include <glm/glm.hpp>
inline std::ostream& operator<<(std::ostream& stream, const glm::vec2 & v) {
    return stream << v.x << ", " << v.y;
}


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
extern bool AssertOnFatal;
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

#define __LOG(level, x) { if ((int)logLevel >= (int)level) { SAC_LOG_PRE logToStream(SAC_LOG_STREAM, level, __FILE__, __LINE__) << x << std::endl; SAC_LOG_POST } }


#define LOGF(x) { __LOG(LogVerbosity::FATAL, x) assert(!AssertOnFatal); }
#define LOGE(x) __LOG(LogVerbosity::ERROR, x)
#define LOGT(x) __LOG(LogVerbosity::TODO, "<--TODO-->" << x)
#define LOGW(x) __LOG(LogVerbosity::WARNING, x)
#define LOGI(x) __LOG(LogVerbosity::INFO, x)
#define LOGV(verbosity, x) { if ((int)logLevel >= ((int)(LogVerbosity::INFO) + verbosity)) { SAC_LOG_PRE vlogToStream(SAC_LOG_STREAM, verbosity, __FILE__, __LINE__) << x << std::endl; SAC_LOG_POST} }

#define LOGF_IF(cond, x) { if ((cond)) LOGF(x) }
#define LOGE_IF(cond, x) { if ((cond)) LOGE(x) }
#define LOGT_IF(cond, x) { if ((cond)) LOGT(x) }
#define LOGW_IF(cond, x) { if ((cond)) LOGW(x) }
#define LOGI_IF(cond, x) { if ((cond)) LOGI(x) }
#define LOGV_IF(verbosity, cond, x) { if ((cond) && (int)logLevel >= (int)LogVerbosity::INFO + verbosity) { SAC_LOG_PRE vlogToStream(SAC_LOG_STREAM, verbosity, __FILE__, __LINE__) << x << std::endl; SAC_LOG_POST} }

#define LOGE_EVERY_N(n, x) {\
    static unsigned __log_count = 0;\
    if ((++__log_count % n) == 0) {\
        LOGE(x) \
    } \
}

#define LOGW_EVERY_N(n, x) {\
    static unsigned __log_count = 0;\
    if ((++__log_count % n) == 0) {\
        LOGW(x) \
    } \
}

#define LOGI_EVERY_N(n, x) {\
    static unsigned __log_count = 0;\
    if ((++__log_count % n) == 0) {\
        LOGI(x) \
    } \
}


#else

#define LOGF(x) {}
#define LOGE(x) {}
#define LOGT(x) {}
#define LOGW(x) {}
#define LOGI(x) {}
#define LOGV(verbosity, x) {}

#define LOGF_IF(cond, x) {}
#define LOGE_IF(cond, x) {}
#define LOGT_IF(cond, x) {}
#define LOGW_IF(cond, x) {}
#define LOGV_IF(verbosity, cond, x) {}

#define LOGE_EVERY_N(n, x) {}
#define LOGW_EVERY_N(n, x) {}

#endif
