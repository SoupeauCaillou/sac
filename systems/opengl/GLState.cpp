#include "GLState.h"

#if SAC_INGAME_EDITORS
#include "../RenderingSystem.h"
#include "util/LevelEditor.h"
#endif

GLState::GLState() {
    viewport.w = 0;
    viewport.h = 0;
    clear.color = Color(0, 0, 0);
    flags.current = 0;
}

void GLState::Viewport::update(int _w, int _h, GLUpdateOption::Enum option) {
    if (_w != w || _h != h || option == GLUpdateOption::Forced) {
        w = _w;
        h = _h;
#if SAC_INGAME_EDITORS
        GL_OPERATION(glViewport(LevelEditor::GameViewPosition().x, LevelEditor::GameViewPosition().y, w, h))
#else
        GL_OPERATION(glViewport(0, 0, w, h))
#endif
    }
}

void GLState::Clear::update(const Color& _color, GLUpdateOption::Enum option) {
    if (_color != color || option == GLUpdateOption::Forced) {
        color = _color;
        GL_OPERATION(glClearColor(color.r, color.g, color.b, color.a))
    }
}

uint32_t GLState::Flags::update(uint32_t bits, GLUpdateOption::Enum option) {
    int bitsChanged = current ^ bits;
    if (option == GLUpdateOption::Forced) {
        bitsChanged = ~0;
    }

    if (bitsChanged & EnableZWriteBit ) {
        GL_OPERATION(glDepthMask(bits & EnableZWriteBit))
    }

    // iff EnableBlendingBit changed
    if (bitsChanged & EnableBlendingBit ) {
        if (bits & EnableBlendingBit) {
            #if SAC_INGAME_EDITORS
            if (!theRenderingSystem.wireframe)
            #endif
            GL_OPERATION(glEnable(GL_BLEND))
        } else {
             GL_OPERATION(glDisable(GL_BLEND))
        }
    }

    // iff EnableColorWriteBit changed
    if (bitsChanged & EnableColorWriteBit ) {
        const bool colorMask = bits & EnableColorWriteBit;
        GL_OPERATION(glColorMask(colorMask, colorMask, colorMask, colorMask))
    }

    current = bits;
    return bitsChanged;
}
