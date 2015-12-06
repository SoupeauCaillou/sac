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

#include <sys/stat.h>
#include <fcntl.h>

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

// TODO: use truecolor https://gist.github.com/XVilka/8346728
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
static bool detect_gdb(void)
{
    char buf[1024];
    int debugger_present = 0;

    int status_fd = open("/proc/self/status", O_RDONLY);
    if (status_fd == -1)
        return 0;

    ssize_t num_read = read(status_fd, buf, sizeof(buf));

    if (num_read > 0)
    {
        static const char TracerPid[] = "TracerPid:";
        char *tracer_pid;

        buf[num_read] = 0;
        tracer_pid    = strstr(buf, TracerPid);
        if (tracer_pid)
            debugger_present = !!atoi(tracer_pid + sizeof(TracerPid) - 1);
    }

    return debugger_present;
}

void initLogColors() {
    enableColorize = !detect_gdb();

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
