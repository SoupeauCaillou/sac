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



#if SAC_INGAME_EDITORS

#include "DebugConsole.h"

#include <algorithm>

#include <cstring>

DebugConsole & DebugConsole::Instance() {
    static DebugConsole _instance;
    return _instance;
}

void DebugConsole::initTW() {
    bar = TwNewBar("Debug_Console");
    TwDefine(" Debug_Console size='400 200' iconified=true valueswidth=250");
}

void DebugConsole::RegisterMethod(const std::string & name, void (*callback)(void*),
    void* storingPlace) {
    LOGF_IF(Instance().name2callback.find(name) != Instance().name2callback.end(), "function " << name << " already registered!");

    Instance().name2callback[name] = callback;

    LOGV(1, "New entry for debug console: " << name);

    TwAddButton(Instance().bar, name.c_str(), (TwButtonCallback)callback, storingPlace, "valueswidth=fit");
}

void DebugConsole::RegisterMethod(const std::string & name, void (*callback)(void*),
    const std::string & argumentName, TwType type, void* storingPlace) {

    TwAddVarRW(Instance().bar, argumentName.c_str(), type, storingPlace, "valueswidth=fit");
    RegisterMethod(name, callback, storingPlace);
}

void DebugConsole::RegisterMethod(const std::string & name, void (*callback)(void*),
    const std::string & argumentName, TwEnumVal* availableArgs, unsigned availableArgsSize, void* storingPlace) {

    TwType type = TwDefineEnum(argumentName.c_str(), availableArgs, availableArgsSize);
    RegisterMethod(name, callback, argumentName, type, storingPlace);
}

#endif

