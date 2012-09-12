#pragma once

#include <string>

enum ProfilePhase {
	BeginEvent,
	EndEvent,
	InstantEvent,
	AsyncStart,
	AsyncFinish
};

void initProfiler();

void addProfilePoint(const std::string& category, const std::string& name, enum ProfilePhase ph);

void saveToFile(const std::string& filename);

#ifdef ENABLE_PROFILING
#define PROFILE(cat, name, phase) addProfilePoint(cat, name, phase);
#else
#define PROFILE(cat, name, phase) 
#endif
	