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



bool AssertOnFatal = true;

#if SAC_ENABLE_LOG

#include "Log.h"

#include "TimeUtil.h"
// #include <iomanip>
// #include <sstream>
#include "util/MurmurHash.h"

char __logLineBuffer[MAX_LINE_LENGTH];

LogVerbosity::Enum logLevel =
#ifdef SAC_ANDROID
    LogVerbosity::INFO;
#else
    LogVerbosity::INFO;
#endif
// std::map<std::string, bool> verboseFilenameFilters;

#if SAC_DESKTOP
// std::stringstream lastLogsSS;
std::recursive_mutex lastLogsMutex;
// std::queue<std::string> lastLogs;
unsigned lastLogsCount = 100;
void writeLastLogs() {
    #if 0
    std::cout.flush();
    std::cerr << "**********************" << std::endl;
    std::cerr << "************ LAST LOGS" << std::endl;
    while (!lastLogs.empty()) {
        std::cerr << lastLogs.front();
        lastLogs.pop();
    }
    std::cerr << "**********************" << std::endl;
    #endif
}
#endif

#if 0
class NullStream : public std::ostream {
public:
        NullStream(std::basic_streambuf<char, std::char_traits<char>> *t = 0) : std::ostream(t) {}

    template<class T>
    std::ostream& operator<<(const T& ) {
        return *this;
    }
};
NullStream slashDevslashNull;
#endif
static const char* enumNames[] ={
    //5 chars length for all
        " F ",
        " E ",
    " T ",
    " W ",
        " I ",
    " V1",
    " V2",
    " V3",
};

#include <cstring>

static const char* keepOnlyFilename(const char* fullPath) {
    #define SIZE 20
    static char filename[SIZE + 1];

    int len = (int) strlen(fullPath);
    int offset = len - SIZE;
    int start = offset < 0 ? 0 : offset;
    sprintf(filename, "%*s", SIZE, &fullPath[start]);
    return filename;
}

static const char* enum2Name(LogVerbosity::Enum t) {
        return enumNames[(int)t];
}

#if SAC_DESKTOP && SAC_LINUX

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

static const char* color[LogVerbosity::COUNT] = {
    "30;41", /* FATAL, default: fg=black, bg=red */
    "30;41", /* ERROR, default: fg=black, bg=red */
    "30;44", /* TODO, default:  fg=black, bg=blue */
    "30;43", /* WARN, default:  fg=black, bg=yellow */
    "30;42", /* INFO, default:  fg=black, bg=green */
    "37;40", /* VERB1, default:  fg=white, bg=black */
    "37;40", /* VERB2, default:  fg=white, bg=black */
    "37;40", /* VERB3, default:  fg=white, bg=black */
};

static const char* colorTs = {
    "30;100", /* fg=black, bg=bright black */
};

static const char* validColors[] = {
    "31", // Red
    "32", // Green
    "33", // Yellow
    "34", // Light Blue
    "35", // Light Purple
    "36", // Light Cyan
    "37" // White
};

const char* pickColorTag(const char* tag) {
    return validColors[Murmur::RuntimeHash(tag) % 7];
}
static bool enableColorize = true;

// from http://stackoverflow.com/questions/3596781/detect-if-gdb-is-running
// gdb apparently opens FD(s) 3,4,5 (whereas a typical prog uses only stdin=0, stdout=1,stderr=2)
static int detect_gdb(void)
{
    int rc = 0;
    FILE *fd = fopen("/tmp", "r");

    if (fileno(fd) > 5)
    {
        rc = 1;
    }

    fclose(fd);
    return 0;//rc;
}

void initLogColors() {
    enableColorize = detect_gdb() == 0;

    // read from env var

}

static const char* ColorPrefix = "\033[";
static const char* ColorReset = "\033[0m";
#endif

int logPrefix(char* out, LogVerbosity::Enum type, const char* file, int line) {
#if SAC_DESKTOP && SAC_LINUX
    if (enableColorize) {
    pid_t tid;

    tid = syscall(SYS_gettid);

    return snprintf(out, MAX_LINE_LENGTH,
        "%s4%cm%d%s "
        "%s%sm%s%s "
        "%s%sm %4.3f %s "
        "%s%sm%s:%3d%s ",

        ColorPrefix, '1' + (2 + tid % 4), tid, ColorReset,
        ColorPrefix, color[type], enum2Name(type), ColorReset,
        ColorPrefix, colorTs, TimeUtil::GetTime(), ColorReset,
        ColorPrefix, pickColorTag(file), keepOnlyFilename(file), line, ColorReset);
    } else
#endif

    // LOGT("colorless logs");
    #if 0
    stream
        << enum2Name(type) << ' '
        << std::fixed << std::setprecision(4)
        << TimeUtil::GetTime() << ' '
        << keepOnlyFilename(file) << ':' << line << ' ';
        return stream;
    #endif
    return 0;
}

#endif
