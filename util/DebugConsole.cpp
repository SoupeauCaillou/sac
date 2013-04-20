#if SAC_INGAME_EDITORS

#include "DebugConsole.h"

#include "base/Log.h"

#include <algorithm>

DebugConsole & DebugConsole::Instance() {
    static DebugConsole _instance;
    return _instance;
}

void DebugConsole::registerMethod(const std::string & name, void (*callback)(void*)) {
    LOGF_IF(name2callback.find(name) != name2callback.end(), "function " << name << " already registered!");

    name2callback[name] = callback;

    LOGI("New entry for debug console: " << name);
    TwAddButton(bar, name.c_str(), (TwButtonCallback)&callback, 0, 0);
}

#endif

