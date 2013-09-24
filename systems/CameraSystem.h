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
#include "RenderingSystem.h"

struct CameraComponent {
    CameraComponent() : fb(DefaultFrameBufferRef), clearColor(0,0,0), enable(false), clear(true), order(0), id(0) {}
    // assume complete draw of FB
    FramebufferRef fb;
    Color clearColor;
    bool enable, clear;
    int order;
    int id;
};

struct TransformationComponent;

#define theCameraSystem CameraSystem::GetInstance()
#define CAMERA(e) theCameraSystem.Get(e)

UPDATABLE_SYSTEM(Camera)

public:
    static bool isDisabled(Entity e);
    static bool sort(Entity e, Entity f);

    // Transform world coords to screen, based on Camera TransformComp
    static glm::vec2 WorldToScreen(const TransformationComponent* tc, const glm::vec2& pos);
    static glm::vec2 ScreenToWorld(const TransformationComponent* tc, const glm::vec2& pos);
};
