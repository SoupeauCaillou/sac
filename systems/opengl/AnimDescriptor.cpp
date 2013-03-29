#include "AnimDescriptor.h"
#include "api/AssetAPI.h"
#include "util/DataFileParser.h"
#include "systems/RenderingSystem.h"

bool AnimDescriptor::load(const FileBuffer& fb, std::string* variables, int varcount) {
    DataFileParser dfp;
    if (!dfp.load(fb)) {
        return false;
    }
    for (int i=0; i<varcount; i++) {
        dfp.defineVariable(variables[2*i], variables[2*i+1]);
    }
    // Parse meta info
    const std::string meta = "meta";
        // speed
    if (dfp.get(meta, "speed", &playbackSpeed, 1, false)) {
        LOGV(1, "animation -> playbackSpeed = " << playbackSpeed)
    } else {
        playbackSpeed = 1;
    }
        // loop count
    loopCount = Interval<int>(-1,-1);
    std::string loop;
    if (dfp.get(meta, "loop", &loop, 1, false)) {
        if (loop == "infinite") {
            loopCount = Interval<int>(-1,-1);
            LOGV(1, "animation -> loopCount = infinite")
        } else if (dfp.get(meta, "loop", &loopCount.t1, 2, false)) {
            LOGV(1, "animation -> loopCount = [" << loopCount.t1 << ", " << loopCount.t2 << ']')
        } else if (dfp.get(meta, "loop", &loopCount.t1, 1, false)) {
            loopCount.t2 = loopCount.t1;
            LOGV(1, "animation -> loopCount = " << loopCount.t1)
        }
    }
        // next anim
    if (dfp.get(meta, "next_anim", &nextAnim, 1, false)) {
        LOGV(1, "animation -> nextAnim = " << nextAnim)
    } else {
        nextAnim = "";
    }
        // wait before next
    if (dfp.get(meta, "wait_before_next_anim", &nextAnimWait.t1, 2, false)) {
        LOGV(1, "animation -> nextAnimWait = [" << nextAnimWait.t1 << ", " << nextAnimWait.t2 << ']')
    } else if (dfp.get(meta, "wait_before_next_anim", &nextAnimWait.t1, 1, false)) {
        nextAnimWait.t2 = nextAnimWait.t1;
        LOGV(1, "animation -> nextAnimWait = " << nextAnimWait.t1)
    } else {
        nextAnimWait.t1 = nextAnimWait.t2 = 0;
    }
        // frame count
    int frameCount = 0;
    if (dfp.get(meta, "num_frames", &frameCount, 1)) {
        LOGV(1, "animation -> frameCount = " << frameCount)

        for (int i=0; i<frameCount; i++) {
            std::stringstream sect;
            sect << "frame" << i;
            std::string section = sect.str();
            std::string texture;
            if (dfp.get(section, "texture", &texture, 1)) {
                LOGV(1, "\t" << section << ':' << texture)
                AnimFrame frame;
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
                LOGE("Missing texture attribute in section '" << section << "'")
                return false;
            }
        }
    }

    return true;
}
