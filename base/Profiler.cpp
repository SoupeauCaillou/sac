#include "Profiler.h"

#ifdef ENABLE_PROFILING
#include "libs/jsoncpp-src-0.6.0-rc2/include/json/json.h"
#include <sys/types.h>
#include <time.h>
#include <stdint.h>
#include <fstream>
#include <glog/logging.h>

#ifdef WINDOWS
void initProfiler() {
	LOG(WARNING) << "todo";
}
void startProfiler() {
	LOG(WARNING) << "todo";
}

void stopProfiler(const std::string& filename) {
	LOG(WARNING) << "todo";
}

void addProfilePoint(const std::string& category, const std::string& name, enum ProfilePhase ph) {
	LOG(WARNING) << "todo";
}

#else


#include <pthread.h>
static Json::Value* root;
static pthread_mutex_t mutex;
static bool started;

void initProfiler() {
	pthread_mutex_init(&mutex, 0);
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
	sample["tid"] = (unsigned)pthread_self();
	sample["ts"] = (unsigned long long int)t1.tv_sec * 1000000 + (unsigned long long int)t1.tv_nsec / 1000;
	sample["ph"] = phaseEnum2String(ph);
	sample["args"] = Json::Value(Json::arrayValue);
	
	pthread_mutex_lock(&mutex);
	(*root)["traceEvents"].append(sample);
	pthread_mutex_unlock(&mutex);
}

void startProfiler() {
	pthread_mutex_lock(&mutex);
	if (started)
		return;
	LOG(INFO) << "Start profiler";
	root->clear();
	started = true;
	pthread_mutex_unlock(&mutex);
}

void stopProfiler(const std::string& filename) {
	pthread_mutex_lock(&mutex);
	if (!started)
		return;
    LOG(INFO) << "Stop profiler, saving to: " << filename;
	std::ofstream out(filename.c_str());
	out << *root;
	started = false;
	pthread_mutex_unlock(&mutex);
}
#endif
#endif
