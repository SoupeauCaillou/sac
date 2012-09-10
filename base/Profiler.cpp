#include "Profiler.h"

#ifdef ENABLE_PROFILING
#include <jsoncpp/json/json.h>
#include <sys/types.h>
#include <time.h>
#include <stdint.h>
#include <fstream>
#include <pthread.h>
Json::Value root;

static inline const char* phaseEnum2String(enum ProfilePhase ph) {
	switch (ph) {
		case BeginEvent: return "B";
		case EndEvent: return "E";
		case InstantEvent: return "I";
		case AsyncStart: return "S";
		case AsyncFinish: return "F";
		default: return "?";
	}
}

void addProfilePoint(const std::string& category, const std::string& name, enum ProfilePhase ph) {
	timespec t1;
	clock_gettime(CLOCK_REALTIME, &t1);	
	
	Json::Value sample;
	sample["cat"] = category;
	sample["name"] = name;
	sample["pid"] = 1;
	sample["tid"] = (unsigned)pthread_self();
	sample["ts"] = (unsigned long long int)t1.tv_sec * 1000 + (unsigned long long int)t1.tv_nsec / 1000000;
	sample["ph"] = phaseEnum2String(ph);
	sample["args"] = Json::Value(Json::arrayValue);
	
	root["traceEvents"].append(sample);
}

void saveToFile(const std::string& filename) {
	std::ofstream out(filename.c_str());
	out << root;
}
#endif
