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
#include "OpenglHelper.h"
#include <glog/logging.h>

void check_GL_errors(const char* context) {
     int maxIterations=10;
    GLenum error;
    while (((error = glGetError()) != GL_NO_ERROR) && maxIterations > 0)
    {
        switch(error)
        {
            case GL_INVALID_ENUM:
                LOG(ERROR) << '[' << maxIterations << "]GL error: '" << context << "' -> GL_INVALID_ENUM"; break;
            case GL_INVALID_VALUE:
                LOG(ERROR) << '[' << maxIterations << "]GL error: '" << context << "' -> GL_INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:
                LOG(ERROR) << '[' << maxIterations << "]GL error: '" << context << "' -> GL_INVALID_OPERATION"; break;
            case GL_OUT_OF_MEMORY:
                LOG(ERROR) << '[' << maxIterations << "]GL error: '" << context << "' -> GL_OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                LOG(ERROR) << '[' << maxIterations << "]GL error: '" << context << "' -> GL_INVALID_FRAMEBUFFER_OPERATION"; break;
            default:
                LOG(ERROR) << '[' << maxIterations << "]GL error: '" << context << "' -> " << error; break;
        }
          maxIterations--;
    }
}