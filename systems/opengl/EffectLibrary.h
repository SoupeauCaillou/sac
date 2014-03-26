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

#include "base/NamedAssetLibrary.h"
#include "api/AssetAPI.h"
#include "OpenglHelper.h"

typedef uint8_t EffectRef;
#define DefaultEffectRef 0


struct Shader {
    GLuint program;
    GLuint uniformMatrix, uniformColorSampler, uniformAlphaSampler, uniformColor, uniformCamera;
#if SAC_USE_VBO
    GLuint uniformUVScaleOffset, uniformRotation, uniformScaleZ;
#endif
};

#define DEFAULT_FRAGMENT "default.fs"
#define DEFAULT_NO_ALPHA_FRAGMENT "default_no_alpha.fs"
#define DEFAULT_NO_TEXTURE_FRAGMENT "default_no_texture.fs"
#define EMPTY_FRAGMENT "empty.fs"

class EffectLibrary : public NamedAssetLibrary<Shader, EffectRef, FileBuffer> {
    protected:
        bool doLoad(const char* name, Shader& out, const EffectRef& ref);

        void doUnload(const Shader& in);

        void doReload(const char* name, const EffectRef& ref);

    public:
        virtual void init(AssetAPI* pAssetAPI, bool pUseDeferredLoading = true);

        const char* asset2FilePrefix() const { return ""; }
        const char* asset2FileSuffix() const { return ""; }

        enum {
            ATTRIB_VERTEX = 0,
            ATTRIB_UV,
            ATTRIB_SCALE,
            NUM_ATTRIBS
        };
};
