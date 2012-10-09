/*
 This file is part of Heriswap.

 @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
 @author Soupe au Caillou - Gautier Pelloux-Prayer

 Heriswap is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, version 3.

 Heriswap is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "AnimationSystem.h"
#include "base/MathUtil.h"

struct AnimationSystem::Anim {
    std::vector<TextureRef> textures;
    float playbackSpeed;
};
    
INSTANCE_IMPL(AnimationSystem);
 
AnimationSystem::AnimationSystem() : ComponentSystemImpl<AnimationComponent>("Animation") { 
    AnimationComponent tc;
    componentSerializer.add(new StringProperty(OFFSET(name, tc)));
}

void AnimationSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(Animation, a, bc)
        AnimIt jt = animations.find(bc->name);
        if (jt == animations.end())
            continue;
        Anim* anim = jt->second;

        if (bc->previousName != bc->name) {
            bc->textureIndex = 0;
            RENDERING(a)->texture = anim->textures[bc->textureIndex];
            bc->accum = 0;
            bc->previousName = bc->name;
        } else {
            bc->accum += dt * anim->playbackSpeed;
    
            while(bc->accum >= 1) {
                bc->textureIndex = (bc->textureIndex + 1) % anim->textures.size();
                RENDERING(a)->texture = anim->textures[bc->textureIndex];
                bc->accum -= 1;
            }
        }
    }
}

void AnimationSystem::registerAnim(const std::string& name, std::vector<TextureRef> textures, float playbackSpeed) {
    assert (animations.find(name) == animations.end());
    Anim* a = new Anim();
    a->textures = textures;
    a->playbackSpeed = playbackSpeed;
    animations[name] = a;
}

void AnimationSystem::registerAnim(const std::string& name, std::string* textureNames, int count, float playbackSpeed) {
    assert (animations.find(name) == animations.end());
    std::vector<TextureRef> textures;
    for (int i=0; i<count; i++) {
        textures.push_back(theRenderingSystem.loadTextureFile(textureNames[i]));
    }
    registerAnim(name, textures, playbackSpeed);
}
