#include "Log.h"
#include "TimeUtil.h"
#include <iomanip>

LogVerbosity::Enum logLevel = LogVerbosity::INFO;

static const char* enumNames[] ={
	"FATAL",
	"ERROR",
	"WARN ",
	"INFO ",
	"VERB "
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

std::ostream& logToStream(std::ostream& stream, LogVerbosity::Enum type, const char* file, int line) {
	stream << std::fixed << std::setprecision(4) << TimeUtil::GetTime() << ' ' << enum2Name(type) << ' ' << keepOnlyFilename(file) << ':' << line << " : ";
	return stream;
}

std::ostream& vlogToStream(std::ostream& stream, int level, const char* file, int line) {
	stream << std::fixed << std::setprecision(4) << TimeUtil::GetTime() << " VERB-" << level << ' ' << keepOnlyFilename(file) << ':' << line << " : ";
	return stream;
}