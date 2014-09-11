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

#include "base/Interval.h"
#include "TextureLibrary.h"
#include <vector>
#include "util/MurmurHash.h"

struct FileBuffer;

class AnimDescriptor {
    public:
        AnimDescriptor();

        bool load(const std::string& ctx, const FileBuffer& fb, std::string* variables = 0, int varcount = 0);

    public:
        struct AnimFrame {
            TextureRef texture;
        };

    public:
        std::vector<AnimFrame> frames;
        float playbackSpeed;
        Interval<int> loopCount;
        hash_t nextAnim;
        Interval<float> nextAnimWait;
};
