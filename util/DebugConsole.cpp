#if SAC_INGAME_EDITORS

#include "DebugConsole.h"

#include <algorithm>

#include <cstring>

DebugConsole & DebugConsole::Instance() {
    static DebugConsole _instance;
    return _instance;
}

void DebugConsole::registerMethodWithOneArg(const std::string & name, const std::string & argumentName, void (*callback)(void*), TwEnumVal* availableArgs, unsigned availableArgsSize, void* storingPlace) {
    LOGF_IF(Instance().name2callback.find(name) != Instance().name2callback.end(), "function " << name << " already registered!");

    Instance().name2callback[name] = callback;

    LOGI("New entry for debug console: " << name);

    TwType type = TwDefineEnum(argumentName.c_str(), availableArgs, availableArgsSize);
    TwAddVarRW(Instance().bar, argumentName.c_str(), type, storingPlace, "");

    TwAddButton(Instance().bar, name.c_str(), (TwButtonCallback)callback, storingPlace, "");
}

void DebugConsole::registerMethodWithoutArg(const std::string & name, void (*callback)(void*)) {
    LOGF_IF(Instance().name2callback.find(name) != Instance().name2callback.end(), "function " << name << " already registered!");

    Instance().name2callback[name] = callback;

    LOGE("New entry for debug console: " << name);

    TwAddButton(Instance().bar, name.c_str(), (TwButtonCallback)callback, 0, "");
}

void DebugConsole::init() {
    bar = TwNewBar("Debug_Console");
    TwDefine(" Debug_Console size='400 200' iconified=true ");
}
#endif

