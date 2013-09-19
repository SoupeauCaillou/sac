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

typedef int EffectRef;
#define DefaultEffectRef -1


struct Shader {
    GLuint program;
    GLuint uniformMatrix, uniformColorSampler, uniformAlphaSampler, uniformColor, uniformCamera;
#if SAC_USE_VBO
    GLuint uniformUVScaleOffset, uniformRotation, uniformScaleZ;
#endif
};

#define DEFAULT_FRAGMENT "default.fs"
#define DEFAULT_NO_ALPHA_FRAGMENT "default_no_alpha.fs"
#define EMPTY_FRAGMENT "empty.fs"

class EffectLibrary : public NamedAssetLibrary<Shader, EffectRef, FileBuffer> {
    protected:
        bool doLoad(const std::string& name, Shader& out, const EffectRef& ref);

        void doUnload(const std::string& name, const Shader& in);

        void doReload(const std::string& name, const EffectRef& ref);

    public:
        virtual void init(AssetAPI* pAssetAPI, bool pUseDeferredLoading = true);

        std::string asset2File(const std::string& asset) const { return asset; }

        enum {
            ATTRIB_VERTEX = 0,
            ATTRIB_UV,
            ATTRIB_SCALE,
            NUM_ATTRIBS
        };
};
