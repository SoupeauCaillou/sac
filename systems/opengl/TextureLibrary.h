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
#include "base/Vector2.h"
#include "util/ImageLoader.h"

#if defined(ANDROID) || defined(EMSCRIPTEN)
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#else
#include <GL/glew.h>
#endif

// Internal representation of a GL texture
struct Texture {
    GLuint id;
    Vector2 size;
};

typedef int TextureRef;

class TextureLibrary : public NamedAssetLibrary<Texture, TextureRef> {
    public:
        bool doLoad(const std::string& name, Texture& out, const TextureRef& ref);

        void doUnload(const std::string& name, const Texture& in);


        void reload(const std::string& name, Texture& out);

        void setPvrFormatSupported(bool s) { pvrFormatSupported = s; }

        void setPkmFormatSupported(bool s) { pkmFormatSupported = s; }

    private:
        ImageDesc parseImageContent(const std::string& filename, const FileBuffer& file, bool isPng) const;

        bool pvrFormatSupported, pkmFormatSupported;
};