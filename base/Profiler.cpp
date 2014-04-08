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
#include <vector>

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
static std::vector<std::string> root;
static std::mutex mutex;
static bool started;

void initProfiler() {
    started = false;
}

static inline const char* phaseEnum2String(enum ProfilePhase ph) {
    switch (ph) {
        case BeginEvent:            return "B";
        case EndEvent:              return "E";
        case CompleteEvent:         return "X";
        case InstantEvent:          return "I";
        case CounterEvent:          return "C";
        case AsyncStartEvent:       return "S";
        case AsyncStepEvent:        return "T";
        case AsyncFinishEvent:      return "F";
        case FlowStartEvent:        return "s";
        case FlowStepEvent:         return "t";
        case FlowFinishEvent:       return "f";
        case MetaDataEvent:         return "M";
        case SampleEvent:           return "P";
        case ObjectCreatedEvent:    return "O";
        case ObjectSnapshotEvent:   return "N";
        case ObjectDestroyedEvent:  return "D";
        default:                    return "?";
    }
}

static inline const char* scopeEnum2String(enum InstantScope scope) {
    switch (scope) {
        case ThreadScope:           return "t";
        case ProcessScope:          return "p";
        case GlobalScope:           return "g";
        default:                    return "?";
    }
}

void addProfilePoint(const std::string& category, const std::string& name, enum ProfilePhase ph, enum InstantScope scope/*=ThreadScope*/, int id/*=0*/) {
    if (!started)
        return;
    timespec t1;
    clock_gettime(CLOCK_REALTIME, &t1);

    int pid = 1;
    unsigned long long int ts = (unsigned long long int)t1.tv_sec * 1000000 + (unsigned long long int)t1.tv_nsec / 1000;
    int dur = 10;
    std::stringstream a;
    a << "{\"name\":\"" << name << "\",";
    a << "\"cat\":\"" << category << "\",";
    a << "\"ph\":\"" << phaseEnum2String(ph) << "\",";
    a << "\"pid\":" << pid << ",";
    a << "\"tid\":" << std::this_thread::get_id() << ",";
    a << "\"ts\":" << ts << ",";
    a << "\"args\":{}";
    if( ph == CompleteEvent )
    {
        a << ",\"dur\":" << dur;
    }
    if( ph == InstantEvent )
    {
        a << ",\"s\":\"" << scopeEnum2String(scope) << "\"";
    }
    if( ph >= AsyncStartEvent || ph <= AsyncFinishEvent )
    {
        a << ",\"id\":" << id;
    }
    a << "}";
    a.flush();

    std::string s = a.str();

    std::unique_lock<std::mutex> lck(mutex);
    root.push_back(s);
}

void startProfiler() {
    std::unique_lock<std::mutex> lck(mutex);
    if (started)
        return;
    LOGI("Start profiler");
    root.clear();
    started = true;
}

void stopProfiler(const char* filename) {
    std::unique_lock<std::mutex> lck(mutex);
    if (!started)
        return;
    LOGI("Stop profiler, saving to: " << filename);
    std::ofstream out(filename);
    out << "{\"traceEvents\": [";
    for(std::vector<std::string>::iterator it=root.begin(); it!=root.end(); ++it)
    {
        if( it != root.begin() )
            out << ",";
        out << *it;
        out.flush();
    }
    out << "]}";

    started = false;
}

#endif
#endif
