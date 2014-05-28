#pragma once

#include "OpenglHelper.h"
#include "../../base/Color.h"

// Bits values choosen in order to get:
//    opaque > alpha-blended
//    z-pre-pass-opaque > opaque
//    z-pre-pass-alpha-blended > alpha-blended
//    opaque > z-pre-pass-alpha-blended

#define EnableZWriteBit       0x1
#define EnableBlendingBit     0x2
#define EnableColorWriteBit   0x4
#define EnableConstantBit     0x8

namespace GLUpdateOption {
    enum Enum {
        IfDirty,
        Forced
    };
}

struct GLState {
    GLState();

    struct Viewport {
        Viewport():w(0),h(0){}
        int w, h;

        void update(int _w, int _h, GLUpdateOption::Enum option = GLUpdateOption::IfDirty);
    } viewport;

    struct Clear {
        Color color;

        void update(const Color& _color, GLUpdateOption::Enum option = GLUpdateOption::IfDirty);
    } clear;

    struct Flags {
        Flags():current(0){}
        uint32_t current;

        uint32_t update(uint32_t bits, GLUpdateOption::Enum option = GLUpdateOption::IfDirty);
    } flags;
};
