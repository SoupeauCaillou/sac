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
#include "base/Log.h"
#include "systems/CameraSystem.h"
#include "systems/CollisionSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"
#include "Draw.h"
#include "util/MurmurHash.h"

static bool showGrid = false;

DebugConsole & DebugConsole::Instance() {
    static DebugConsole _instance;
    return _instance;
}

static void setShowGridCallback(const void* value, void* ) {
    showGrid = *(static_cast<const bool*> (value));

    if (showGrid) {
        glm::vec2 topLeft = glm::vec2(-3 * theRenderingSystem.screenW, 3 * theRenderingSystem.screenH);
        topLeft.x = floor(topLeft.x); topLeft.y = ceil(topLeft.y);
        for (int i=0; i<=ceil(theRenderingSystem.screenW * 6); i++) {
            Draw::Vec2(Murmur::Hash(__FILE__), topLeft + glm::vec2(i, 0), glm::vec2(0, -theRenderingSystem.screenH * 6), Color(0.2f, 0.2f, 0.2f, 0.2f));

            for (int j=1; j<=4; j++) {
                Draw::Vec2(Murmur::Hash(__FILE__), topLeft + glm::vec2(i + j * 0.2, 0), glm::vec2(0, -theRenderingSystem.screenH * 6), Color(0.2f, 0.2f, 0.2f, 0.08f));
            }
        }

        for (int i=0; i<=ceil(theRenderingSystem.screenH * 6); i++) {
            Draw::Vec2(Murmur::Hash(__FILE__), topLeft + glm::vec2(0, -i), glm::vec2(theRenderingSystem.screenW * 6, 0), Color(0.2f, 0.2f, 0.2f, 0.2f));

            for (int j=1; j<=4; j++) {
                Draw::Vec2(Murmur::Hash(__FILE__), topLeft + glm::vec2(0, -i - j * 0.2), glm::vec2(theRenderingSystem.screenW * 6, 0), Color(0.2f, 0.2f, 0.2f, 0.08f));
            }
        }

    } else {
        Draw::Clear(Murmur::Hash(__FILE__));
    }
}

static void getShowGridCallback(void* value, void* ) {
    *(static_cast<bool*>(value)) = showGrid;
}

void DebugConsole::initTW() {
    bar = TwNewBar("Debug_Console");
    TwDefine(" Debug_Console size='400 200' iconified=true ");

    TwAddVarCB(bar, "Show grid", TW_TYPE_BOOLCPP, setShowGridCallback, getShowGridCallback, 0, 0 );
    // Rendering debug
    TwAddVarRW(bar, "Show opaque", TW_TYPE_BOOLCPP, &theRenderingSystem.highLight.opaque, "group=Rendering");
    TwAddVarRW(bar, "Show non-opaque", TW_TYPE_BOOLCPP, &theRenderingSystem.highLight.nonOpaque, "group=Rendering");
    TwAddVarRW(bar, "Show runtime opaque", TW_TYPE_BOOLCPP, &theRenderingSystem.highLight.runtimeOpaque, "group=Rendering");
    TwAddVarRW(bar, "Show z prepass", TW_TYPE_BOOLCPP, &theRenderingSystem.highLight.zPrePass, "group=Rendering");
    TwAddVarRW(bar, "Wireframe", TW_TYPE_BOOLCPP, &theRenderingSystem.wireframe, "group=Rendering");

    // Collision debug
    TwAddVarRW(bar, "Show debug", TW_TYPE_BOOLCPP, &theCollisionSystem.showDebug, "group=Collision");
    TwAddVarRW(bar, "Max raycast per sec", TW_TYPE_FLOAT, &theCollisionSystem.maximumRayCastPerSec, "group=Collision");

}

void DebugConsole::RegisterMethod(const std::string & name, void (*callback)(void*),
    void* storingPlace) {
    LOGF_IF(Instance().name2callback.find(name) != Instance().name2callback.end(), "function " << name << " already registered!");

    Instance().name2callback[name] = callback;

    LOGV(1, "New entry for debug console: " << name);

    TwAddButton(Instance().bar, name.c_str(), (TwButtonCallback)callback, storingPlace, "group=Game");
}

void DebugConsole::RegisterMethod(const std::string & name, void (*callback)(void*),
    const std::string & argumentName, TwType type, void* storingPlace) {

    TwAddVarRW(Instance().bar, argumentName.c_str(), type, storingPlace, "group=Game");
    RegisterMethod(name, callback, storingPlace);
}

void DebugConsole::RegisterMethod(const std::string & name, void (*callback)(void*),
    const std::string & argumentName, TwEnumVal* availableArgs, unsigned availableArgsSize, void* storingPlace) {

    TwType type = TwDefineEnum(argumentName.c_str(), availableArgs, availableArgsSize);
    RegisterMethod(name, callback, argumentName, type, storingPlace);
}

#endif

