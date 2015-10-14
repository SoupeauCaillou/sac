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



#include "AnimationSystem.h"
#include "TransformationSystem.h"
#include "RenderingSystem.h"
#include "opengl/AnimDescriptor.h"
#include "util/SerializerProperty.h"
#include "base/EntityManager.h"

static void applyFrameToEntity(Entity e, const AnimationComponent*, const AnimDescriptor::AnimFrame& frame) {
    LOGV(2, "animation: " << theEntityManager.entityName(e) << ": new frame = '" << INV_HASH(frame.texture) << "'");

    // hum..RENDERING(e)->show = (frame.texture != InvalidTextureRef);
    RENDERING(e)->texture = frame.texture;
}

INSTANCE_IMPL(AnimationSystem);

AnimationSystem::AnimationSystem() : ComponentSystemImpl<AnimationComponent>(HASH("Animation", 0x3050a3d5)) {
    AnimationComponent tc;
    componentSerializer.add(new Property<hash_t>(HASH("name", 0x195267c7), OFFSET(name, tc)));
    componentSerializer.add(new Property<float>(HASH("playback_speed", 0xe564d97a), OFFSET(playbackSpeed, tc), 0.001f));
    componentSerializer.add(new Property<float>(HASH("accum", 0xdfadabe5), OFFSET(accum, tc), 0.001f));
    componentSerializer.add(new Property<float>(HASH("wait_accum", 0x3abca19d), OFFSET(waitAccum, tc), 0.001f));
    componentSerializer.add(new Property<int>(HASH("loop_count", 0xb84aa73d), OFFSET(loopCount, tc)));
    componentSerializer.add(new Property<int>(HASH("frame_index", 0x5f66ffd2), OFFSET(frameIndex, tc)));
}

AnimationSystem::~AnimationSystem() {
    for(auto it=animations.begin(); it!=animations.end(); ++it) {
        delete it->second;
    }
    animations.clear();
}

void AnimationSystem::DoUpdate(float dt) {
    FOR_EACH_ENTITY_COMPONENT(Animation, a, bc)
        if (!bc->name)
            continue;

        auto jt = animations.find(bc->name);
        if (jt == animations.end()) {
            LOGW("Animation '" << INV_HASH(bc->name) << "' not found. " << animations.size() << " defined animation(s):");
            #if SAC_ENABLE_LOG
            for (auto an: animations) {
                LOGW("   '" << INV_HASH(an.first) << "' - " << an.second->frames.size() << " frames");
            }
            #endif
            LOGF_IF(animations.empty(), "Weird, no animations loaded");
            continue;
        }
        const AnimDescriptor* anim = jt->second;

        if (bc->previousName != bc->name) {
            bc->frameIndex = 0;
            applyFrameToEntity(a, bc, anim->frames[bc->frameIndex]);
            bc->accum = 0;
            bc->previousName = bc->name;
            bc->loopCount = anim->loopCount.random();
        } else if (bc->playbackSpeed > 0) {
            if (bc->waitAccum > 0) {
                bc->waitAccum -= dt;
                if (bc->waitAccum <= 0) {
                    bc->waitAccum = 0;
                    RENDERING(a)->show = true;
                }
            } else {
                bc->accum += dt * anim->playbackSpeed * bc->playbackSpeed;
            }

            while(bc->accum >= 1) {
                bool lastImage = (bc->frameIndex == (int)anim->frames.size() - 1);
                if (lastImage) {
                    if (bc->loopCount != 0) {
                        bc->frameIndex = 0;
                        bc->loopCount--;

                        /*if (bc->loopCount == 0) {
                            bc->name = bc->previousName = "";
                            break;
                        }*/
                    } else if (anim->nextAnim) {
                        if ((bc->waitAccum = anim->nextAnimWait.random()) > 0)
                            RENDERING(a)->show = false;
                        bc->name = anim->nextAnim;
                        // hum, maybe bc->waitAccum should be -= accum leftover..
                        bc->accum = 0;
                        break;
                    }
                } else {
                    bc->frameIndex++;
                }
                applyFrameToEntity(a, bc, anim->frames[bc->frameIndex]);
                // RENDERING(a)->texture = anim->frames[bc->frameIndex].texture;
                bc->accum -= 1;
            }
        }
    END_FOR_EACH()
}

void AnimationSystem::loadAnim(AssetAPI* assetAPI, const std::string& name, const std::string& filename, std::string* variables, int varcount) {
    const std::string fileN("anim/" + filename + ".anim");
    AnimDescriptor* desc = new AnimDescriptor;
    FileBuffer file = assetAPI->loadAsset(fileN);
    if (file.size) {
        if (desc->load(fileN, file, variables, varcount)) {
            animations.insert(std::make_pair(Murmur::RuntimeHash(name.c_str()), desc));
        } else {
            LOGE("Invalid animation file: " << filename << ".anim");
            delete desc;
        }
    } else {
        if (!file.data)
            return;
        LOGE("Empty animation file: " << filename << ".anim");
        delete desc;
    }
    delete[] file.data;
}
