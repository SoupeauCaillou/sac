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



#include "AnimDescriptor.h"
#include "api/AssetAPI.h"
#include "util/DataFileParser.h"
#include "systems/RenderingSystem.h"
#include "util/MurmurHash.h"


AnimDescriptor::AnimDescriptor() : playbackSpeed(1.0f), nextAnim(0) {
    loopCount.t1 = loopCount.t2 = 0;
    nextAnimWait.t1 = nextAnimWait.t2 = 0.0f;
}

bool AnimDescriptor::load(const std::string& ctx, const FileBuffer& fb, std::string* variables, int varcount) {
    DataFileParser dfp;
    if (!dfp.load(fb, ctx)) {
        return false;
    }
    LOGV(1, "Loading: " << ctx);
    for (int i=0; i<varcount; i++) {
        dfp.defineVariable(variables[2*i], variables[2*i+1]);
    }
    // Parse meta info
    const hash_t meta = HASH("meta", 0x9e8e687a);
        // speed
    if (dfp.get(meta, "speed", &playbackSpeed, 1, false)) {
        LOGV(1, "  -> playbackSpeed = " << playbackSpeed);
    } else {
        playbackSpeed = 1;
    }
        // loop count
    loopCount = Interval<int>(-1,-1);
    std::string loop;
    if (dfp.get(meta, "loop", &loop, 1, false)) {
        if (loop == "infinite") {
            loopCount = Interval<int>(-1,-1);
            LOGV(1, "  -> loopCount = infinite");
        } else if (dfp.get(meta, "loop", &loopCount.t1, 2, false)) {
            LOGV(1, "  -> loopCount = [" << loopCount.t1 << ", " << loopCount.t2 << ']');
        } else if (dfp.get(meta, "loop", &loopCount.t1, 1, false)) {
            loopCount.t2 = loopCount.t1;
            LOGV(1, "  -> loopCount = " << loopCount.t1);
        }
    }
        // next anim
    std::string next;
    if (dfp.get(meta, "next_anim", &next, 1, false)) {
        LOGV(1, "  -> nextAnim = " << next);
        nextAnim = Murmur::RuntimeHash(next.c_str());
        LOGF_IF(nextAnim == 0, "Hash value 0 is reserved, please change anim '" << ctx << "' name");
    } else {
        nextAnim = 0;
    }
        // wait before next
    if (dfp.get(meta, "wait_before_next_anim", &nextAnimWait.t1, 2, false)) {
        LOGV(1, "  -> nextAnimWait = [" << nextAnimWait.t1 << ", " << nextAnimWait.t2 << ']');
    } else if (dfp.get(meta, "wait_before_next_anim", &nextAnimWait.t1, 1, false)) {
        nextAnimWait.t2 = nextAnimWait.t1;
        LOGV(1, "  -> nextAnimWait = " << nextAnimWait.t1);
    } else {
        nextAnimWait.t1 = nextAnimWait.t2 = 0;
    }
        // frame count
    int frameCount = 0;
    if (dfp.get(meta, "num_frames", &frameCount, 1)) {
        LOGV(1, "  -> frameCount = " << frameCount);

        for (int i=0; i<frameCount; i++) {
            std::stringstream sect;
            sect << "frame" << i;
            hash_t section = Murmur::RuntimeHash(sect.str().c_str());
            std::string texture;
            if (dfp.get(section, "texture", &texture, 1)) {
                LOGV(1, "    - " << section << ':' << texture);
                AnimFrame frame;
                frame.attributesCount = 0;
                if (texture.empty())
                    frame.texture = InvalidTextureRef;
                else
                    frame.texture = Murmur::RuntimeHash(texture.c_str());

                // load other keys as attributes
                int keys = dfp.sectionSize(section);
                for (int j=0; j<keys; j++) {
                    std::string name;
                    float values[MAX_VALUE_PER_ATTRIBUTE];
                    int valueCount = 0;
                    if ((valueCount = dfp.get(section, j, name, values, MAX_VALUE_PER_ATTRIBUTE))) {
                        hash_t attrH = Murmur::RuntimeHash(name.c_str());
                        if (attrH == HASH("texture", 0x3d4e3ff8)) {
                            continue;
                        } else {
                            frame.attributes[frame.attributesCount].id = attrH;
                            frame.attributes[frame.attributesCount].count = valueCount;
                            for (int k=0; k<valueCount; k++) {
                                frame.attributes[frame.attributesCount].f[k] = values[k];
                            }
                            ++frame.attributesCount;
                        }
                    }

                    if (frame.attributesCount >= MAX_ATTR_PER_FRAME) {
                        break;
                    }
                }

                frames.push_back(frame);
            } else {
                LOGF("Missing texture attribute in section '" << INV_HASH(section) << "'");
                return false;
            }


        }
    }

    return true;
}
