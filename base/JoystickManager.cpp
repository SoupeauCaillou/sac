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



#include "JoystickManager.h"

#include "base/Log.h"
#include "base/TimeUtil.h"

#if ! SAC_ANDROID
#include <SDL/SDL.h>
#endif
JoystickManager* JoystickManager::instance = 0;


JoystickManager* JoystickManager::Instance() {
	if (instance == 0) instance = new JoystickManager();
	return instance;
}

int JoystickManager::eventSDL(void* inEvent) {
    #if ! SAC_ANDROID
        auto event = (SDL_Event*)inEvent;
        if (!event) {
            return 0;
        }
        if (event->type == SDL_JOYAXISMOTION) {
            int joystick = event->jaxis.which;
            int pad = event->jaxis.axis / 2;
            if (event->jaxis.axis % 2 == 0) {
                joysticks[joystick].lastDirection[pad].x = event->jaxis.value / 32768.;
            } else {
                joysticks[joystick].lastDirection[pad].y = - event->jaxis.value / 32768.;                
            }
            LOGV(2, "SDL_JOYAXISMOTION: direction=" << joysticks[joystick].lastDirection[pad] << " axis=" << (int)event->jaxis.axis << " value=" << event->jaxis.value);
            return 1;
        } else if (event->type == SDL_JOYBUTTONDOWN) {
            int joystick = event->jaxis.which;
            LOGV(2, "SDL_JOYBUTTONDOWN: " << (int)event->jaxis.axis);

            int button = (int)event->jaxis.axis;

            joysticks[joystick].lastClickTime[button] = TimeUtil::GetTime();
            if (joysticks[joystick].clicked[button]) {
                joysticks[joystick].clicked[button] = false;
                joysticks[joystick].doubleclicked[button] = true;
            } else {
                joysticks[joystick].clicked[button] = true;
            }
            return 1;
        } else if (event->type == SDL_JOYBUTTONUP) {
            int joystick = event->jaxis.which;
            LOGV(2, "SDL_JOYBUTTONUP: " << (int)event->jaxis.axis);

            int button = (int)event->jaxis.axis;

            if (TimeUtil::GetTime() - joysticks[joystick].lastClickTime[button] > 0.05) {
                joysticks[joystick].clicked[button] = false;
                joysticks[joystick].doubleclicked[button] = false;
            }
            return 1;
        }
    #endif
    return 0;
}

void JoystickManager::Update(float) {
    #if ! SAC_ANDROID
        unsigned newCount = SDL_NumJoysticks();

        if (joysticks.size() != newCount) {
            LOGW("joysticks.size() changed from " << joysticks.size() << " to " << SDL_NumJoysticks());
        
            SDL_JoystickEventState(SDL_ENABLE);
            for (unsigned j = joysticks.size(); j < newCount; ++j) {
                LOGI("Opening joystick "<< j << "...");
                SDL_JoystickOpen(j);
            }
            joysticks.resize(newCount);
        }
    #endif
}

void JoystickManager::resetDoubleClick(int idx, JoystickButton::Enum btn) {
    joysticks[idx].lastClickTime[btn] = 0;
}
