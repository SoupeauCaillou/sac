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
#pragma once

#if !DISABLE_SWYPE_SYSTEM
#include "System.h"

class VibrateAPI;

namespace SwypeIdleState {
    enum Enum {
        Used,
        Halted,
        Animating,
        GoingBackToHalt,
    };
}
struct SwypeButtonComponent {
    SwypeButtonComponent()
        : mouseOver(false), clicked(false), lastPos(0.0f), idlePos(0.0f),
          animationPlaying(SwypeIdleState::Halted), enabled(false),
          vibration(0.035f) {}

    ////// READ ONLY variables
    // States of button
    bool mouseOver;
    bool clicked, touchStartOutside;
    // Last position when entity was clicked
    glm::vec2 lastPos;
    // idle position
    glm::vec2 idlePos;
    //
    SwypeIdleState::Enum animationPlaying;
    //
    float activeIdleTime;
    ////// END OF READ ONLY variables

    ////// READ/WRITE variables
    // if true, entity is clickable
    bool enabled;
    // Speed of button
    glm::vec2 speed;
    // Swype direction
    glm::vec2 finalPos;
    ////// END OF READ/WRITE variables

    ////// WRITE ONLY variables
    // Vibration on clicked
    float vibration;
    // animated button
    bool animated;
    ////// END OF WRITE ONLY variables
};

#define theSwypeButtonSystem SwypeButtonSystem::GetInstance()
#if SAC_DEBUG
#define SWYPEBUTTON(actor)                                                     \
    theSwypeButtonSystem.Get(actor, true, __FILE__, __LINE__)
#else
#define SWYPEBUTTON(actor) theSwypeButtonSystem.Get(actor)
#endif
UPDATABLE_SYSTEM(SwypeButton)

public:
VibrateAPI* vibrateAPI;

private:
void UpdateSwypeButton(float dt,
                       Entity entity,
                       SwypeButtonComponent* comp,
                       bool touching,
                       const glm::vec2& touchPos);
}
;
#endif
