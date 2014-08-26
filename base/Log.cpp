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
#include <iomanip>
#include <sstream>

LogVerbosity::Enum logLevel =
#ifdef SAC_ANDROID
    LogVerbosity::INFO;
#else
    LogVerbosity::INFO;
#endif
std::map<std::string, bool> verboseFilenameFilters;

#if SAC_DESKTOP
std::stringstream lastLogsSS;
std::queue<std::string> lastLogs;
unsigned lastLogsCount = 100;
void writeLastLogs() {
    std::cout.flush();
    std::cerr << "**********************" << std::endl;
    std::cerr << "************ LAST LOGS" << std::endl;
    while (!lastLogs.empty()) {
        std::cerr << lastLogs.front();
        lastLogs.pop();
    }
    std::cerr << "**********************" << std::endl;
}
#endif

class NullStream : public std::ostream {
public:
	NullStream(std::basic_streambuf<char, std::char_traits<char>> *t = 0) : std::ostream(t) {}

    template<class T>
    std::ostream& operator<<(const T& ) {
        return *this;
    }
};
NullStream slashDevslashNull;

static const char* enumNames[] ={
    //5 chars length for all
	"FATAL",
	"ERROR",
    "TODO ",
    "WARN ",
	"INFO ",
    "VERB1",
    "VERB2",
    "VERB3",
};

static const char* keepOnlyFilename(const char* fullPath) {
	const char* result = fullPath;
	const char* ptr = fullPath;
	while (*ptr++ != '\0') {
		if (*ptr == '\\' || *ptr == '/') result = ptr + 1;
	}
	return result;
}

static const char* enum2Name(LogVerbosity::Enum t) {
	return enumNames[(int)t];
}

int logHeaderLength(const char* file, int line) {
    static std::stringstream ss;
    ss.str("");
    logToStream(ss, LogVerbosity::INFO, file, line);
    return ss.str().size();
}

std::ostream& logToStream(std::ostream& stream, LogVerbosity::Enum type, const char* file, int line) {
	stream << std::fixed << std::setprecision(4) << TimeUtil::GetTime() << ' ' << enum2Name(type) << ' ' << keepOnlyFilename(file) << ':' << line << " : ";
	return stream;
}

std::ostream& vlogToStream(std::ostream& stream, int level, const char* file, int line) {
    const char* trimmed = keepOnlyFilename(file);
    auto it = verboseFilenameFilters.find(trimmed);
    if (it == verboseFilenameFilters.end())
        verboseFilenameFilters.insert(std::make_pair(trimmed, true));
    else if (!it->second)
        return slashDevslashNull;
	stream << std::fixed << std::setprecision(4) << TimeUtil::GetTime() << " VERB-" << level << ' ' << trimmed << ':' << line << " : ";
	return stream;
}

#endif
