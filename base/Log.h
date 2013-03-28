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
	};
}

extern LogVerbosity::Enum logLevel;
extern std::map<std::string, bool> verboseFilenameFilters;

std::ostream& logToStream(std::ostream& stream, LogVerbosity::Enum type, const char* file, int line);
std::ostream& vlogToStream(std::ostream& stream, int level, const char* file, int line);

#ifdef SAC_ANDROID
#define LOGF(x) {}
#define LOGE(x) {}
#define LOGW(x) {}
#define LOGI(x) {}
#define LOGV(verbosity, x) {}

#define LOGF_IF(cond, x) {}
#define LOGE_IF(cond, x) {}
#define LOGW_IF(cond, x) {}
#define LOGI_IF(cond, x) {}
#define LOGV_IF(verbosity, cond, x) { }
#else
#include <iostream>
#define LOGF(x) { logToStream(std::cout, LogVerbosity::FATAL, __FILE__, __LINE__) << x << std::endl; }
#define LOGE(x) { logToStream(std::cout, LogVerbosity::ERROR, __FILE__, __LINE__) << x << std::endl; }
#define LOGW(x) { if (logLevel >= (int)LogVerbosity::WARNING) logToStream(std::cout, LogVerbosity::WARNING, __FILE__, __LINE__) << x << std::endl; }
#define LOGI(x) { if (logLevel >= (int)LogVerbosity::INFO) logToStream(std::cout, LogVerbosity::INFO, __FILE__, __LINE__) << x << std::endl; }
#define LOGV(verbosity, x) { if (logLevel >= ((int)(LogVerbosity::INFO) + verbosity)) {vlogToStream(std::cout, verbosity, __FILE__, __LINE__) << x << std::endl;} }

#define LOGF_IF(cond, x) { if ((cond)) { logToStream(std::cout, LogVerbosity::FATAL, __FILE__, __LINE__) << x << std::endl;}}
#define LOGE_IF(cond, x) { if ((cond)) { logToStream(std::cout, LogVerbosity::ERROR, __FILE__, __LINE__) << x << std::endl;}}
#define LOGW_IF(cond, x) { if ((cond) && logLevel >= (int)LogVerbosity::WARNING) { logToStream(std::cout, LogVerbosity::WARNING, __FILE__, __LINE__) << x << std::endl; } }
#define LOGI_IF(cond, x) { if ((cond) && logLevel >= (int)LogVerbosity::INFO) { logToStream(std::cout, LogVerbosity::INFO, __FILE__, __LINE__) << x << std::endl; } }
#define LOGV_IF(verbosity, cond, x) { if ((cond) && logLevel >= (int)LogVerbosity::INFO + verbosity) { vlogToStream(std::cout, verbosity, __FILE__, __LINE__) << x << std::endl; } }

#define LOG_EVERY_N(log, n, x) {\
    static int __log_count = 0;\
    if (!__log_count) {\
        __log_count = n;\
        log(x);\
    } else {\
        --__log_count;\
    }\
}


#endif
