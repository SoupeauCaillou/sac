#pragma once

#undef ERROR
#include <ostream>
#include <map>
namespace LogVerbosity {
	enum Enum {
		FATAL,
		ERROR,
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

#ifdef SAC_ANDROID
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

#define LOGF(x) { SAC_LOG_PRE logToStream(SAC_LOG_STREAM, LogVerbosity::FATAL, __FILE__, __LINE__) << x << std::endl; SAC_LOG_POST }
#define LOGE(x) { SAC_LOG_PRE logToStream(SAC_LOG_STREAM, LogVerbosity::ERROR, __FILE__, __LINE__) << x << std::endl; SAC_LOG_POST }
#define LOGW(x) { if (logLevel >= (int)LogVerbosity::WARNING) { SAC_LOG_PRE logToStream(SAC_LOG_STREAM, LogVerbosity::WARNING, __FILE__, __LINE__) << x << std::endl; SAC_LOG_POST } }
#define LOGI(x) { if (logLevel >= (int)LogVerbosity::INFO) { SAC_LOG_PRE logToStream(SAC_LOG_STREAM, LogVerbosity::INFO, __FILE__, __LINE__) << x << std::endl; SAC_LOG_POST } }
#define LOGV(verbosity, x) { if (logLevel >= ((int)(LogVerbosity::INFO) + verbosity)) { SAC_LOG_PRE vlogToStream(SAC_LOG_STREAM, verbosity, __FILE__, __LINE__) << x << std::endl; SAC_LOG_POST} }

#define LOGF_IF(cond, x) { if ((cond)) { SAC_LOG_PRE logToStream(SAC_LOG_STREAM, LogVerbosity::FATAL, __FILE__, __LINE__) << x << std::endl; SAC_LOG_POST}}
#define LOGE_IF(cond, x) { if ((cond)) { SAC_LOG_PRE logToStream(SAC_LOG_STREAM, LogVerbosity::ERROR, __FILE__, __LINE__) << x << std::endl; SAC_LOG_POST}}
#define LOGW_IF(cond, x) { if ((cond) && logLevel >= (int)LogVerbosity::WARNING) { SAC_LOG_PRE logToStream(SAC_LOG_STREAM, LogVerbosity::WARNING, __FILE__, __LINE__) << x << std::endl; SAC_LOG_POST} }
#define LOGI_IF(cond, x) { if ((cond) && logLevel >= (int)LogVerbosity::INFO) { SAC_LOG_PRE logToStream(SAC_LOG_STREAM, LogVerbosity::INFO, __FILE__, __LINE__) << x << std::endl; SAC_LOG_POST} }
#define LOGV_IF(verbosity, cond, x) { if ((cond) && logLevel >= (int)LogVerbosity::INFO + verbosity) { SAC_LOG_PRE vlogToStream(SAC_LOG_STREAM, verbosity, __FILE__, __LINE__) << x << std::endl; SAC_LOG_POST} }

#define LOG_EVERY_N(log, n, x) {\
    static int __log_count = 0;\
    if (!__log_count) {\
        __log_count = n;\
        log(x);\
    } else {\
        --__log_count;\
    }\
}
