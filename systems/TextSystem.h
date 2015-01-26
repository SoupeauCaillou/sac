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

#pragma once

#include "System.h"
#include "base/Color.h"

#include "opengl/TextureLibrary.h"

#include <vector>

struct TextComponent {
    const static float LEFT;
    const static float CENTER;
    const static float RIGHT;

    const static int IsANumberBit = 1 << 0;
    const static int AdjustHeightToFillWidthBit = 1 << 1;
    const static int MultiLineBit = 1 << 2;

    TextComponent()
        : text(""), color(Color(1.f)), charHeight(1.),
          fontName(HASH("typo", 0x5a18f4a9)), positioning(CENTER), show(false),
          flags(0), cameraBitMask(1), maxLineToUse(-1)
#if SAC_INGAME_EDITORS
          ,
          highLight(false)
#endif
    {
        caret.show = false;
        caret.speed = caret.dt = 0;
        blink.offDuration = blink.onDuration = blink.accum = 0;
    }

    std::string text;
    Color color;

    union {
        float charHeight;
        float maxCharHeight;
    };

    hash_t fontName;
    float positioning;
    bool show;
    int flags;
    struct {
        bool show;
        float speed;
        float dt;
    } caret;
    struct {
        float offDuration;
        float onDuration;
        float accum;
    } blink;
    unsigned cameraBitMask;
    int maxLineToUse;

#if SAC_INGAME_EDITORS
    bool highLight;
#endif
};

#define theTextSystem TextSystem::GetInstance()
#if SAC_WINDOWS
#undef TEXT
#endif
#if SAC_DEBUG
#define TEXT(e) theTextSystem.Get(e, true, __FILE__, __LINE__)
#else
#define TEXT(e) theTextSystem.Get(e)
#endif

UPDATABLE_SYSTEM(Text)

public:
~TextSystem();

void Delete(Entity e) override;
void registerFont(const char* name,
                  const std::map<uint32_t, float>& charH2Wratio);

float computeTextComponentWidth(TextComponent* trc) const;

struct CharInfo {
    float h2wRatio;
    TextureRef texture;
};
struct FontDesc {
    uint32_t highestUnicode;
    CharInfo* entries;
};

private:
std::vector<Entity> renderingEntitiesPool;
std::map<hash_t, FontDesc> fontRegistry;
}
;
