#include "Profiler.h"

#if SAC_ENABLE_PROFILING
#include "libs/jsoncpp-src-0.6.0-rc2/include/json/json.h"
#include <sys/types.h>
#include <time.h>
#include <stdint.h>
#include <fstream>
#include "Log.h"
#include <sstream>

#if SAC_WINDOWS || SAC_DARWIN
void initProfiler() {
	LOGW("todo")
}
void startProfiler() {
	LOGW("todo")
}

void stopProfiler(const std::string& filename) {
	LOGW("todo")
}

void addProfilePoint(const std::string& category, const std::string& name, enum ProfilePhase ph) {
	LOGW("todo")
}

#else


#include <thread>
#include <mutex>
static Json::Value* root;
static std::mutex mutex;
static bool started;

void initProfiler() {
	root = new Json::Value;
	started = false;
}

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
	if (!started)
		return;
	timespec t1;
	clock_gettime(CLOCK_REALTIME, &t1);

	Json::Value sample;
	sample["cat"] = category;
	sample["name"] = name;
	sample["pid"] = 1;
    std::stringstream a;
    a << std::this_thread::get_id();
	sample["tid"] = a.str();
	sample["ts"] = (unsigned long long int)t1.tv_sec * 1000000 + (unsigned long long int)t1.tv_nsec / 1000;
	sample["ph"] = phaseEnum2String(ph);
	sample["args"] = Json::Value(Json::arrayValue);

	mutex.lock();
	(*root)["traceEvents"].append(sample);
	mutex.unlock();
}

void startProfiler() {
	mutex.lock();
	if (started)
		return;
	LOGI("Start profiler")
	root->clear();
	started = true;
	mutex.unlock();
}

void stopProfiler(const std::string& filename) {
	mutex.lock();
	if (!started)
		return;
    LOGI("Stop profiler, saving to: " << filename)
	std::ofstream out(filename.c_str());
	out << *root;
	started = false;
	mutex.unlock();
}
#endif
#endif
