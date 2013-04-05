/*
    This file is part of sac.

    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer

    Heriswap is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3.

    Heriswap is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
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

class EffectLibrary : public NamedAssetLibrary<Shader, EffectRef, FileBuffer> {
    protected:
        bool doLoad(const std::string& name, Shader& out, const EffectRef& ref);

        void doUnload(const std::string& name, const Shader& in);

        void doReload(const std::string& name, const EffectRef& ref);

    public:
        virtual void init(AssetAPI* pAssetAPI);

        enum {
            ATTRIB_VERTEX = 0,
            ATTRIB_UV,
            ATTRIB_POS_ROT,
            ATTRIB_SCALE,
            NUM_ATTRIBS
        };
};
