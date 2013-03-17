#include "AnimationSystem.h"
#include "TransformationSystem.h"
#include "opengl/AnimDescriptor.h"

static void applyFrameToEntity(Entity e, const AnimationComponent* animComp, const AnimDescriptor::AnimFrame& frame) {
    RENDERING(e)->texture = frame.texture;
    LOG_IF(WARNING, animComp->subPart.size() != frame.transforms.size()) << "Animation entity subpart count " << animComp->subPart.size() << " is different from frame transform count " << frame.transforms.size();
    for (unsigned i=0; i<frame.transforms.size() && i<animComp->subPart.size(); i++) {
        TransformationComponent* tc = TRANSFORM(animComp->subPart[i]);
        const AnimDescriptor::AnimFrame::Transform& trans = frame.transforms[i];
        tc->position = trans.position;
        tc->size = trans.size;
        tc->rotation = trans.rotation;
    }
}

INSTANCE_IMPL(AnimationSystem);

AnimationSystem::AnimationSystem() : ComponentSystemImpl<AnimationComponent>("Animation") {
    AnimationComponent tc;
    componentSerializer.add(new StringProperty(OFFSET(name, tc)));
    componentSerializer.add(new Property<float>(OFFSET(playbackSpeed, tc), 0.001));
    componentSerializer.add(new Property<int>(OFFSET(loopCount, tc)));
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
        AnimDescriptor* anim = jt->second;

        if (bc->previousName != bc->name) {
            bc->frameIndex = 0;
            applyFrameToEntity(a, bc, anim->frames[bc->frameIndex]);
            // RENDERING(a)->texture = anim->frames[bc->frameIndex].texture;
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
                bool lastImage = (bc->frameIndex == (int)anim->frames.size() - 1);
                if (lastImage) {
                    if (bc->loopCount != 0) {
                        bc->frameIndex = 0;
                        bc->loopCount--;
                    } else if (!anim->nextAnim.empty()) {
                        if ((bc->waitAccum = anim->nextAnimWait.random()) > 0)
                            RENDERING(a)->hide = true;
                        bc->name = anim->nextAnim;
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
    }
}

void AnimationSystem::loadAnim(const std::string& name, const std::string& filename, std::string* variables, int varcount) {
    FileBuffer file = assetAPI->loadAsset("anim/" + filename + ".anim");
    if (file.size) {
        AnimDescriptor* desc = new AnimDescriptor;
        if (desc->load(file, variables, varcount)) {
            animations.insert(std::make_pair(name, desc));
        } else {
            LOG(ERROR) << "Invalid animation file: " << filename << ".anim";
            delete desc;
        }
    } else {
        LOG(ERROR) << "Empty animation file: " << filename << ".anim";
    }
    delete[] file.data;
}

#ifdef INGAME_EDITORS
void AnimationSystem::addEntityPropertiesToBar(Entity entity, TwBar* bar) {
    AnimationComponent* tc = Get(entity, false);
    if (!tc) return;
    TwAddVarRW(bar, "name", TW_TYPE_STDSTRING, &tc->name, "group=Animation");
    TwAddVarRO(bar, "previousName", TW_TYPE_STDSTRING, &tc->previousName, "group=Animation");
    TwAddVarRO(bar, "accum", TW_TYPE_FLOAT, &tc->accum, "group=Animation");
    TwAddVarRW(bar, "playbackSpeed", TW_TYPE_FLOAT, &tc->playbackSpeed, "group=Animation");
    TwAddVarRW(bar, "loopCount", TW_TYPE_INT32, &tc->loopCount, "group=Animation");
    TwAddVarRO(bar, "textureIndex", TW_TYPE_INT32, &tc->frameIndex, "group=Animation");
    TwAddVarRO(bar, "waitAccum", TW_TYPE_FLOAT, &tc->waitAccum, "group=Animation");
}
#endif