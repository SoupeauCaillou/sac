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



#include "Profiler.h"

#if SAC_ENABLE_PROFILING
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
// static Json::Value* root;
static std::mutex mutex;
static bool started;

void initProfiler() {
	// root = new Json::Value;
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

void addProfilePoint(const std::string& /*category*/, const std::string& /*name*/, enum ProfilePhase /*ph*/) {
	if (!started)
		return;
	timespec t1;
	clock_gettime(CLOCK_REALTIME, &t1);

	LOGT("Json library removed (incompatible with sublimeclang)");
	// // Json::Value sample;
	// sample["cat"] = category;
	// sample["name"] = name;
	// sample["pid"] = 1;
 //    std::stringstream a;
 //    a << std::this_thread::get_id();
	// sample["tid"] = a.str();
	// sample["ts"] = (unsigned long long int)t1.tv_sec * 1000000 + (unsigned long long int)t1.tv_nsec / 1000;
	// sample["ph"] = phaseEnum2String(ph);
	// // sample["args"] = Json::Value(Json::arrayValue);

	// std::unique_lock<std::mutex> lck(mutex);
	// (*root)["traceEvents"].append(sample);
}

void startProfiler() {
	std::unique_lock<std::mutex> lck(mutex);
	if (started)
		return;
	LOGI("Start profiler");
	// root->clear();
	started = true;
}

void stopProfiler(const std::string& /*filename*/) {
	std::unique_lock<std::mutex> lck(mutex);
	if (!started)
		return;
    // LOGI("Stop profiler, saving to: " << filename);
    LOGT("Not saving anything, need to replace the lib");
	// std::ofstream out(filename.c_str());
	// out << *root;
	started = false;
}
#endif
#endif
