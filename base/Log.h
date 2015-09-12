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

// to handle vec2 operator<<
// #include <ostream>
// #include <sstream>
// #include <iomanip>
// #include <map>
#include <vector>

#include <glm/glm.hpp>

#define MAX_LINE_LENGTH 1024

extern char __logLineBuffer[MAX_LINE_LENGTH];
#if 0
inline std::ostream& operator<<(std::ostream& stream, const glm::vec2& v) {
    return stream << v.x << ", " << v.y;
}
template <class T>
inline std::ostream& operator<<(std::ostream& stream, const std::vector<T>& v) {
    size_t s = v.size();
    stream << "vector[" << s << "] = {";
    if (s > 0) {
        for (size_t i = 0; i < s - 1; i++) { stream << v[i] << ','; }
        stream << v[s - 1];
    }
    stream << "}";
    return stream;
}
#endif

#define __(var) #var << ':' << var

#undef ERROR

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

int logPrefix(char* ptr,
                    LogVerbosity::Enum type,
                      const char* file,
                      int line);

#include <signal.h>
#include <string>
#if SAC_ANDROID
#include <stdio.h>
#include <android/log.h>
static const android_LogPriority level2prio[]{ANDROID_LOG_FATAL,
                                              ANDROID_LOG_ERROR,
                                              ANDROID_LOG_WARN,
                                              ANDROID_LOG_WARN,
                                              ANDROID_LOG_INFO,
                                              ANDROID_LOG_VERBOSE,
                                              ANDROID_LOG_VERBOSE};
#endif


#if SAC_WINDOWS
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif

struct LogFmt {
    const char* cFormat;
    LogFmt(const char* fmt = NULL) : cFormat(fmt) {}
};

struct MySimpleStream {
    size_t position;
    LogFmt formatOverride;

    MySimpleStream(int p) : position(p) {}

    template<class T>
    const char* typeToFormat() {
        // default implem
        if (std::is_enum<T>::value) return "%d";
        if (std::is_integral<T>::value) return "%d";
        if (std::is_pointer<T>::value) return "%p";
        if (std::is_array<T>::value) return "%p";
        return "%d";
    }

    /* for c-types */
    template<class T>
    MySimpleStream& operator<<(const T& t) {
        position += snprintf(
            &__logLineBuffer[position], MAX_LINE_LENGTH,
            formatOverride.cFormat ? formatOverride.cFormat : typeToFormat<T>(),
            t);
        return *this;
    }

    template<int N>
    MySimpleStream& operator<<(const char (&t)[N]) {
        position += snprintf(
            &__logLineBuffer[position], MAX_LINE_LENGTH,
            formatOverride.cFormat ? formatOverride.cFormat : "%s",
            t);
        return *this;
    }

};

template<>
inline const char* MySimpleStream::typeToFormat<int>() { return "%d"; }
template<>
inline const char* MySimpleStream::typeToFormat<float>() { return "%.3f"; }
template<>
inline const char* MySimpleStream::typeToFormat<double>() { return "%.3lf"; }
template<>
inline const char* MySimpleStream::typeToFormat<char*>() { return "%s"; }
template<>
inline const char* MySimpleStream::typeToFormat<const char*>() { return "%s"; }
template<>
inline const char* MySimpleStream::typeToFormat<char>() { return "%c"; }


/* overload for c++ class */
template<>
inline MySimpleStream& MySimpleStream::operator<< <glm::vec2>(const glm::vec2& v) {
    position += snprintf(&__logLineBuffer[position], MAX_LINE_LENGTH, "{%.3f, %.3f}", v.x, v.y);
    return *this;
}

template<>
inline MySimpleStream& MySimpleStream::operator<< <std::string>(const std::string& s) {
    position += snprintf(&__logLineBuffer[position], MAX_LINE_LENGTH, "%s", s.c_str());
    return *this;
}

template<>
inline MySimpleStream& MySimpleStream::operator<< <LogFmt>(const LogFmt& v) {
    this->formatOverride = v;
    return *this;
}

#if SAC_ANDROID
#define __PRINT_LOG_LINE __android_log_print(level2prio[logLevel], "sac", "%s", __logLineBuffer);
#else
#define __PRINT_LOG_LINE printf("%s\n", __logLineBuffer);
#endif

#define __LOG(level, x)                                                        \
    do {                                                                       \
        if ((int)logLevel >= (int)level) {                                     \
            MySimpleStream _____str(                                           \
                logPrefix(__logLineBuffer, level, __FILE__, __LINE__));        \
            _____str << x;                                                     \
            __logLineBuffer[MAX_LINE_LENGTH - 1] = '\0';                       \
            __PRINT_LOG_LINE                                                   \
            if (level == LogVerbosity::FATAL && AssertOnFatal) {               \
                raise(SIGABRT);                                                \
            }                                                                  \
        }                                                                      \
    } while (false)

#define __LOG_WHILE(level, x)                                                  \
    PRAGMA_WARNING(warning(disable : 4127))                                    \
    do { __LOG(level, x); } while (false)

#define __LOG_IF_WHILE(cond, level, x)                                         \
    PRAGMA_WARNING(warning(disable : 4127))                                    \
    do {                                                                       \
        if (cond) __LOG(level, x);                                             \
    } while (false)

#define __LOG_EVERY_N_WHILE(cond, n, x)                                        \
    PRAGMA_WARNING(warning(disable : 4127))                                    \
    do {                                                                       \
        static unsigned __log_count = 0;                                       \
        if ((++__log_count % n) == 0) { __LOG(cond, x); }                      \
    } while (false)

#if SAC_ENABLE_LOG
#define LOG_USAGE_ONLY(x) x


#define LOGF(x) __LOG_WHILE(LogVerbosity::FATAL, x)
#define LOGE(x) __LOG_WHILE(LogVerbosity::ERROR, x)
#define LOGT(x) __LOG_WHILE(LogVerbosity::TODO, "<--TODO-->" << x)
#define LOGW(x) __LOG_WHILE(LogVerbosity::WARNING, x)
#define LOGI(x) __LOG_WHILE(LogVerbosity::INFO, x)
#define LOGV(verbosity, x)                                                     \
    __LOG_WHILE((LogVerbosity::Enum)((int)LogVerbosity::INFO + verbosity), x)

#define LOGF_IF(cond, x) __LOG_IF_WHILE(cond, LogVerbosity::FATAL, x)
#define LOGE_IF(cond, x) __LOG_IF_WHILE(cond, LogVerbosity::ERROR, x)
#define LOGT_IF(cond, x) __LOG_IF_WHILE(cond, LogVerbosity::TODO, x)
#define LOGW_IF(cond, x) __LOG_IF_WHILE(cond, LogVerbosity::WARNING, x)
#define LOGI_IF(cond, x) __LOG_IF_WHILE(cond, LogVerbosity::INFO, x)
#define LOGV_IF(verbosity, cond, x)                                            \
    __LOG_IF_WHILE(                                                            \
        cond, (LogVerbosity::Enum)((int)LogVerbosity::INFO + verbosity), x)

#define LOGE_EVERY_N(n, x) __LOG_EVERY_N_WHILE(LogVerbosity::ERROR, n, x)
#define LOGW_EVERY_N(n, x) __LOG_EVERY_N_WHILE(LogVerbosity::WARNING, n, x)
#define LOGI_EVERY_N(n, x) __LOG_EVERY_N_WHILE(LogVerbosity::INFO, n, x)
#define LOGT_EVERY_N(n, x) __LOG_EVERY_N_WHILE(LogVerbosity::TODO, n, x)
#else
#define LOG_USAGE_ONLY(x)

#define LOGF(x)                                                                \
    do { raise(SIGABRT); } while (false)
#define LOGE(x)                                                                \
    do {                                                                       \
    } while (false)
#define LOGT(x)                                                                \
    do {                                                                       \
    } while (false)
#define LOGW(x)                                                                \
    do {                                                                       \
    } while (false)
#define LOGI(x)                                                                \
    do {                                                                       \
    } while (false)
#define LOGV(verbosity, x)                                                     \
    do {                                                                       \
    } while (false)

#define LOGF_IF(cond, x)                                                       \
    do {                                                                       \
        if (cond) raise(SIGABRT);                                              \
    } while (false)
#define LOGE_IF(cond, x)                                                       \
    do {                                                                       \
    } while (false)
#define LOGT_IF(cond, x)                                                       \
    do {                                                                       \
    } while (false)
#define LOGW_IF(cond, x)                                                       \
    do {                                                                       \
    } while (false)
#define LOGI_IF(cond, x)                                                       \
    do {                                                                       \
    } while (false)
#define LOGV_IF(verbosity, cond, x)                                            \
    do {                                                                       \
    } while (false)

#define LOGE_EVERY_N(n, x)                                                     \
    do {                                                                       \
    } while (false)
#define LOGW_EVERY_N(n, x)                                                     \
    do {                                                                       \
    } while (false)
#define LOGI_EVERY_N(n, x)                                                     \
    do {                                                                       \
    } while (false)
#define LOGT_EVERY_N(n, x)                                                     \
    do {                                                                       \
    } while (false)

#endif

