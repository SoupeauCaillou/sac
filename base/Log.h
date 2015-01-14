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

#include <base/SacDefs.h>

#if SAC_DESKTOP && SAC_LINUX
void initLogColors();
#endif

//to handle vec2 operator<<
#include <ostream>
#include <sstream>
#include <iomanip>
#include <vector>

#include <glm/glm.hpp>
inline std::ostream& operator<<(std::ostream& stream, const glm::vec2 & v) {
    return stream << v.x << ", " << v.y;
}
template<class T>
inline std::ostream& operator<<(std::ostream& stream, const std::vector<T>& v) {
    size_t s = v.size();
    stream << "vector[" << s << "] = {";
    if (s > 0) {
        for (size_t i = 0; i < s - 1; i++) {
            stream << v[i] << ',';
        }
        stream << v[s - 1];
    }
    stream << "}";
    return stream;
}

#define __(var) #var <<':' << var

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
        VERBOSE2,
        VERBOSE3,
        COUNT
    };
}

extern LogVerbosity::Enum logLevel;
extern std::map<std::string, bool> verboseFilenameFilters;
#if SAC_DESKTOP
#include <queue>
#include <mutex>
extern std::stringstream lastLogsSS;
extern std::recursive_mutex lastLogsMutex;
extern std::queue<std::string> lastLogs;
extern unsigned lastLogsCount;
extern void writeLastLogs();
#endif
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
#else /* if SAC_ANDROID */
#include <iostream>
#define SAC_LOG_PRE
#define SAC_LOG_STREAM std::cout
#define SAC_LOG_POST
#endif

#if SAC_WINDOWS
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

#if SAC_DESKTOP
#define __LOG(level, x) \
    do {\
        { \
        lastLogsMutex.lock(); \
        lastLogsSS.str(""); lastLogsSS.clear(); \
        logToStream(lastLogsSS, level, __FILE__, __LINE__) << x << std::endl; \
        lastLogs.push(lastLogsSS.str()); \
        while (lastLogs.size() > lastLogsCount) lastLogs.pop(); \
        lastLogsMutex.unlock();\
        } \
        if ((int)logLevel >= (int)level) {\
            SAC_LOG_PRE \
            logToStream(SAC_LOG_STREAM, level, __FILE__, __LINE__) << x << std::endl; \
            SAC_LOG_POST \
            \
            if (level == LogVerbosity::FATAL && AssertOnFatal) { \
                lastLogsMutex.lock(); \
                writeLastLogs(); \
                raise(SIGABRT); \
            } \
        } \
    } while (false)
#else
#define __LOG(level, x) \
    do {\
        if ((int)logLevel >= (int)level) {\
            SAC_LOG_PRE \
            logToStream(SAC_LOG_STREAM, level, __FILE__, __LINE__) << x << std::endl; \
            SAC_LOG_POST \
            \
            if (level == LogVerbosity::FATAL && AssertOnFatal) { \
                raise(SIGABRT); \
            } \
        } \
    } while (false)
#endif

#define __LOG_WHILE(level, x) \
    PRAGMA_WARNING(warning(disable: 4127)) \
    do { \
        __LOG(level, x); \
    } while (false)

#define __LOG_IF_WHILE(cond, level, x) \
    PRAGMA_WARNING(warning(disable: 4127)) \
    do { \
        if (cond) \
            __LOG(level, x); \
    } while (false)

#define __LOG_EVERY_N_WHILE(cond, n, x) \
    PRAGMA_WARNING(warning(disable: 4127)) \
    do { \
        static unsigned __log_count = 0; \
        if ((++__log_count % n) == 0) { \
            __LOG(cond, x); \
        } \
    } while (false)

#if SAC_ENABLE_LOG
    #define LOG_USAGE_ONLY(x) x

    #define LOG_OFFSET() logHeaderLength(__FILE__,__LINE__)

    #define LOGF(x) __LOG_WHILE(LogVerbosity::FATAL, x)
    #define LOGE(x) __LOG_WHILE(LogVerbosity::ERROR, x)
    #define LOGT(x) __LOG_WHILE(LogVerbosity::TODO, "<--TODO-->" << x)
    #define LOGW(x) __LOG_WHILE(LogVerbosity::WARNING, x)
    #define LOGI(x) __LOG_WHILE(LogVerbosity::INFO, x)
    #define LOGV(verbosity, x) __LOG_WHILE((LogVerbosity::Enum)((int)LogVerbosity::INFO + verbosity), x)

    #define LOGF_IF(cond, x) __LOG_IF_WHILE(cond, LogVerbosity::FATAL, x)
    #define LOGE_IF(cond, x) __LOG_IF_WHILE(cond, LogVerbosity::ERROR, x)
    #define LOGT_IF(cond, x) __LOG_IF_WHILE(cond, LogVerbosity::TODO, x)
    #define LOGW_IF(cond, x) __LOG_IF_WHILE(cond, LogVerbosity::WARNING, x)
    #define LOGI_IF(cond, x) __LOG_IF_WHILE(cond, LogVerbosity::INFO, x)
    #define LOGV_IF(verbosity, cond, x) __LOG_IF_WHILE(cond, (LogVerbosity::Enum)((int)LogVerbosity::INFO + verbosity), x)

    #define LOGE_EVERY_N(n, x) __LOG_EVERY_N_WHILE(LogVerbosity::ERROR, n, x)
    #define LOGW_EVERY_N(n, x) __LOG_EVERY_N_WHILE(LogVerbosity::WARNING, n, x)
    #define LOGI_EVERY_N(n, x) __LOG_EVERY_N_WHILE(LogVerbosity::INFO, n, x)
    #define LOGT_EVERY_N(n, x) __LOG_EVERY_N_WHILE(LogVerbosity::TODO, n, x)
#else
    #define LOG_USAGE_ONLY(x)

    #define LOG_OFFSET() 0
    #define LOGF(x) do {  raise(SIGABRT); } while (false)
    #define LOGE(x) do {} while(false)
    #define LOGT(x) do {} while(false)
    #define LOGW(x) do {} while(false)
    #define LOGI(x) do {} while(false)
    #define LOGV(verbosity, x) do {} while(false)

    #define LOGF_IF(cond, x) do { if (cond) raise(SIGABRT); } while (false)
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
