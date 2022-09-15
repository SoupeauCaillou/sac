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

#if SAC_ANDROID || SAC_EMSCRIPTEN
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <EGL/egl.h>
#elif SAC_IOS
#include <OpenGLES/ES2/gl.h>
#else
#include <GL/glew.h>
#endif

#if SAC_DEBUG
#define CHECK_GL_ERROR 1
#endif

void check_GL_errors(const char* context);

#ifdef CHECK_GL_ERROR
#define GL_OPERATION(x)        \
    do {                       \
        (x);                   \
        check_GL_errors(#x);   \
    } while (false);
#else
#define GL_OPERATION(x) (x);
#endif
