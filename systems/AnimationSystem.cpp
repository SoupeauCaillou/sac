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
    Interval<int> loopCount;
    std::string nextAnim;
    Interval<float> nextAnimWait;
};
    
INSTANCE_IMPL(AnimationSystem);
 
AnimationSystem::AnimationSystem() : ComponentSystemImpl<AnimationComponent>("Animation") { 
    AnimationComponent tc;
    componentSerializer.add(new StringProperty(OFFSET(name, tc)));
    componentSerializer.add(new Property(OFFSET(playbackSpeed, tc), sizeof(float)));
    componentSerializer.add(new Property(OFFSET(loopCount, tc), sizeof(int)));
}

AnimationSystem::~AnimationSystem() {
    for(AnimIt it=animations.begin(); it!=animations.end(); ++it) {
        delete it->second;
    }
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
            bc->loopCount = anim->loopCount.random();
        } else {
            if (bc->waitAccum > 0) {
                bc->waitAccum -= dt;
                if (bc->waitAccum <= 0) {
                    bc->waitAccum = 0;
                    RENDERING(a)->hide = false;
                }
            } else {
                bc->accum += dt * anim->playbackSpeed * bc->playbackSpeed;
            }

            while(bc->accum >= 1) {
                bool lastImage = (bc->textureIndex == (int)anim->textures.size() - 1);
                if (lastImage) {
                    if (bc->loopCount != 0) {
                        bc->textureIndex = 0;
                        bc->loopCount--;
                    } else if (!anim->nextAnim.empty()) {
                        if ((bc->waitAccum = anim->nextAnimWait.random()) > 0)
                            RENDERING(a)->hide = true;
                        bc->name = anim->nextAnim;
                        break;
                    }
                } else {
                    bc->textureIndex++;
                }
                RENDERING(a)->texture = anim->textures[bc->textureIndex];
                bc->accum -= 1;
            }
        }
    }
}

void AnimationSystem::registerAnim(const std::string& name, std::vector<TextureRef> textures, float playbackSpeed, Interval<int> loopCount, const std::string& nextAnim, Interval<float> nextAnimWait) {
    assert (animations.find(name) == animations.end());
    Anim* a = new Anim();
    a->textures = textures;
    a->playbackSpeed = playbackSpeed;
    a->loopCount = loopCount;
    a->nextAnim = nextAnim;
    a->nextAnimWait = nextAnimWait;
    animations[name] = a;
}

void AnimationSystem::registerAnim(const std::string& name, std::string* textureNames, int count, float playbackSpeed, Interval<int> loopCount, const std::string& next, Interval<float> nextAnimWait) {
    assert (animations.find(name) == animations.end());
    std::vector<TextureRef> textures;
    for (int i=0; i<count; i++) {
        textures.push_back(theRenderingSystem.loadTextureFile(textureNames[i]));
    }
    registerAnim(name, textures, playbackSpeed, loopCount, next, nextAnimWait);
}
