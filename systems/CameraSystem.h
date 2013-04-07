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

#include "System.h"
#include "RenderingSystem.h"

struct CameraComponent {
    CameraComponent() : fb(DefaultFrameBufferRef), clearColor(0,0,0), enable(false), order(0), id(0) {}
    // assume complete draw of FB
    FramebufferRef fb;
    Color clearColor;
    bool enable;
    int order;
    int id;
};

#define theCameraSystem CameraSystem::GetInstance()
#define CAMERA(e) theCameraSystem.Get(e)

UPDATABLE_SYSTEM(Camera)

public:
    static bool isDisabled(Entity e);
    static bool sort(Entity e, Entity f);
};
