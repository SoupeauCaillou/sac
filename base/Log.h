#pragma once

#undef ERROR
#include <ostream>
namespace LogVerbosity {
	enum Enum {
		FATAL,
		ERROR,
		WARNING,
		INFO,
	};
}

extern LogVerbosity::Enum logLevel;

std::ostream& logToStream(std::ostream& stream, LogVerbosity::Enum type, const char* file, int line);
std::ostream& vlogToStream(std::ostream& stream, int level, const char* file, int line);

#ifdef ANDROID
#error TODO
#else
#include <iostream>
#define LOGF(x) { logToStream(std::cout, LogVerbosity::FATAL, __FILE__, __LINE__) << x << std::endl; }
#define LOGE(x) { logToStream(std::cout, LogVerbosity::ERROR, __FILE__, __LINE__) << x << std::endl; }
#define LOGW(x) { if (logLevel >= (int)LogVerbosity::WARNING) logToStream(std::cout, LogVerbosity::WARNING, __FILE__, __LINE__) << x << std::endl; }
#define LOGI(x) { if (logLevel >= (int)LogVerbosity::INFO) logToStream(std::cout, LogVerbosity::INFO, __FILE__, __LINE__) << x << std::endl; }
#define LOGV(verbosity, x) { if (logLevel > ((int)(LogVerbosity::INFO) + verbosity)) {vlogToStream(std::cout, verbosity, __FILE__, __LINE__) << x << std::endl;} }

#define LOGF_IF(cond, x) { if ((cond)) { logToStream(std::cout, LogVerbosity::FATAL, __FILE__, __LINE__) << x << std::endl;}}
#define LOGE_IF(cond, x) { if ((cond)) { logToStream(std::cout, LogVerbosity::ERROR, __FILE__, __LINE__) << x << std::endl;}}
#define LOGW_IF(cond, x) { if ((cond) && logLevel >= (int)LogVerbosity::WARNING) { logToStream(std::cout, LogVerbosity::WARNING, __FILE__, __LINE__) << x << std::endl; } }
#define LOGI_IF(cond, x) { if ((cond) && logLevel >= (int)LogVerbosity::INFO) { logToStream(std::cout, LogVerbosity::INFO, __FILE__, __LINE__) << x << std::endl; } }
#define LOGV_IF(verbosity, cond, x) { if ((cond) && logLevel > (int)LogVerbosity::INFO + verbosity) { vlogToStream(std::cout, verbosity, __FILE__, __LINE__) << x << std::endl; } }

#endif
