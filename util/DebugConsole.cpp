#if SAC_INGAME_EDITORS

#include "DebugConsole.h"

#include "base/Log.h"

#include <algorithm>

DebugConsole & DebugConsole::Instance() {
    static DebugConsole _instance;
    return _instance;
}

void DebugConsole::registerMethod(const std::string & name, void (*callback)(std::string*)) {
    LOGF_IF(name2callback.find(name) != name2callback.end(), "function " << name << " already registered!");

    name2callback[name] = callback;
}

void DebugConsole::invoke(const std::string & name, std::string* args) {
    LOGF_IF(name2callback.find(name) == name2callback.end(), "function " << name << " does not exist!");

    (name2callback[name])(args);
}

#endif

