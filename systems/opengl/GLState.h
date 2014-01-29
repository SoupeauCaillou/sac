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


struct GLState {
    struct Viewport {
        int w, h;

        void update(int _w, int _h);
    } viewport;

    struct Clear {
        Color color;

        void update(const Color& _color);
    } clear;

    struct Flags {
        uint32_t current;

        uint32_t update(uint32_t bits);
    } flags;
};