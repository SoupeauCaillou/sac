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

#include "base/Entity.h"
#include <vector>
#include <functional>

namespace Fading {
        enum Enum {
                In,
                Out,
                OutIn
        };
}

class FaderHelper {
        public:
                FaderHelper();

                void init(Entity camera);

                void registerFadingOutEntity(Entity e);

                void registerFadingOutCallback(std::function<void ()> fdCb);

                void registerFadingInEntity(Entity e);

                void registerFadingInCallback(std::function<void ()> fdCb);

                void start(Fading::Enum type, float duration);

                void clearFadingEntities();

                bool update(float dt);

                float getDuration() const { return duration; }

        private:
                Entity fadingEntity;
                float duration;
                float accum;
                Fading::Enum type;

                std::vector<Entity> fadingOut;
                std::vector<Entity> fadingIn;
                std::vector<std::function<void ()>> fadingOutCallbacks;
                std::vector<std::function<void ()>> fadingInCallbacks;
};
