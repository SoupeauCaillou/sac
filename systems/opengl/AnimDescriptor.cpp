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

bool AnimDescriptor::load(const std::string& ctx, const FileBuffer& fb, std::string* variables, int varcount) {
    DataFileParser dfp;
    if (!dfp.load(fb, ctx)) {
        return false;
    }
    for (int i=0; i<varcount; i++) {
        dfp.defineVariable(variables[2*i], variables[2*i+1]);
    }
    // Parse meta info
    const std::string meta = "meta";
        // speed
    if (dfp.get(meta, "speed", &playbackSpeed, 1, false)) {
        LOGV(1, "animation -> playbackSpeed = " << playbackSpeed);
    } else {
        playbackSpeed = 1;
    }
        // loop count
    loopCount = Interval<int>(-1,-1);
    std::string loop;
    if (dfp.get(meta, "loop", &loop, 1, false)) {
        if (loop == "infinite") {
            loopCount = Interval<int>(-1,-1);
            LOGV(1, "animation -> loopCount = infinite");
        } else if (dfp.get(meta, "loop", &loopCount.t1, 2, false)) {
            LOGV(1, "animation -> loopCount = [" << loopCount.t1 << ", " << loopCount.t2 << ']');
        } else if (dfp.get(meta, "loop", &loopCount.t1, 1, false)) {
            loopCount.t2 = loopCount.t1;
            LOGV(1, "animation -> loopCount = " << loopCount.t1);
        }
    }
        // next anim
    if (dfp.get(meta, "next_anim", &nextAnim, 1, false)) {
        LOGV(1, "animation -> nextAnim = " << nextAnim);
    } else {
        nextAnim = "";
    }
        // wait before next
    if (dfp.get(meta, "wait_before_next_anim", &nextAnimWait.t1, 2, false)) {
        LOGV(1, "animation -> nextAnimWait = [" << nextAnimWait.t1 << ", " << nextAnimWait.t2 << ']');
    } else if (dfp.get(meta, "wait_before_next_anim", &nextAnimWait.t1, 1, false)) {
        nextAnimWait.t2 = nextAnimWait.t1;
        LOGV(1, "animation -> nextAnimWait = " << nextAnimWait.t1);
    } else {
        nextAnimWait.t1 = nextAnimWait.t2 = 0;
    }
        // frame count
    int frameCount = 0;
    if (dfp.get(meta, "num_frames", &frameCount, 1)) {
        LOGV(1, "animation -> frameCount = " << frameCount);

        for (int i=0; i<frameCount; i++) {
            std::stringstream sect;
            sect << "frame" << i;
            std::string section = sect.str();
            std::string texture;
            if (dfp.get(section, "texture", &texture, 1)) {
                LOGV(1, "\t" << section << ':' << texture);
                AnimFrame frame;
                if (texture.empty())
                    frame.texture = InvalidTextureRef;
                else
                    frame.texture = theRenderingSystem.loadTextureFile(texture);

                int subEntityIndex = 0;
                do {
                    std::stringstream transformS;
                    transformS << "entity_transform_" << subEntityIndex;

                    AnimFrame::Transform tr;
                    if (dfp.get(section, transformS.str(), &tr.position.x, 5, false)) {
                        frame.transforms.push_back(tr);
                        subEntityIndex++;
                    } else {
                        break;
                    }
                } while (true);
                frames.push_back(frame);
            } else {
                LOGF("Missing texture attribute in section '" << section << "'");
                return false;
            }
        }
    }

    return true;
}
