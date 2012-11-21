/*
 This file is part of libsac.

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
#include "LevelEditor.h"
#include "IntersectionUtil.h"
#include "../base/EntityManager.h"
#include "../base/TouchInputManager.h"
#include "../base/PlacementHelper.h"    
#include "../systems/TransformationSystem.h"
#include "../systems/RenderingSystem.h"
#include <GL/glfw.h>

struct LevelEditor::LevelEditorDatas {
    Entity over;
    Entity selected;
};

static void select(Entity e) {
    RENDERING(e)->effectRef = theRenderingSystem.loadEffectFile("selected.fs");
}
static void deselect(Entity e) {
    RENDERING(e)->effectRef = DefaultEffectRef;
}

LevelEditor::LevelEditor() {
    datas = new LevelEditorDatas();
}

void LevelEditor::tick(float dt) {
    if (glfwGetMouseButton(GLFW_MOUSE_BUTTON_1) == GLFW_RELEASE) {
        if (datas->over)
            RENDERING(datas->over)->effectRef = DefaultEffectRef;
        
        int x, y;
        glfwGetMousePos(&x, &y);
        Vector2 windowPos(x / (float)PlacementHelper::WindowWidth - 0.5, 0.5 - y / (float)PlacementHelper::WindowHeight), position(Vector2::Zero);
        for (unsigned i=0; i<theRenderingSystem.cameras.size(); i++) {
            const RenderingSystem::Camera& cam = theRenderingSystem.cameras[i];
            if (IntersectionUtil::pointRectangle(windowPos, cam.screenPosition, cam.screenSize) && cam.enable) {
                position = cam.worldPosition + windowPos * cam.worldSize;
                break;
            }
        }
        std::vector<Entity> entities = theRenderingSystem.RetrieveAllEntityWithComponent();
            float nearest = 10000;
            for (unsigned i=0; i<entities.size(); i++) {
                if (entities[i] == datas->selected)
                    continue;
                if (RENDERING(entities[i])->hide)
                    continue;
                if (IntersectionUtil::pointRectangle(position, TRANSFORM(entities[i])->worldPosition, TRANSFORM(entities[i])->size)) {
                    float d = Vector2::DistanceSquared(position, TRANSFORM(entities[i])->worldPosition);
                    if (d < nearest) {
                        datas->over = entities[i];
                        nearest = d;
                    }
                }
            }
            if (datas->over)
                RENDERING(datas->over)->effectRef = theRenderingSystem.loadEffectFile("over.fs");
    } else {
        if (datas->over) {
            if (datas->selected)
                deselect(datas->selected);
            datas->selected = datas->over;
            select(datas->selected);
            datas->over = 0;
        }
    }

    if (datas->selected) {
        static int prevWheel = 0;
        int wheel = glfwGetMouseWheel();
        int diff = wheel - prevWheel;
        if (diff) {
            bool shift = glfwGetKey( GLFW_KEY_LSHIFT );
            bool ctrl = glfwGetKey( GLFW_KEY_LCTRL );
            
            if (!shift && !ctrl) {
                TRANSFORM(datas->selected)->rotation += 2 * diff * dt;
            } else {
                if (shift) {
                    TRANSFORM(datas->selected)->size.X *= (1 + 1 * diff * dt); 
                }
                if (ctrl) {
                    TRANSFORM(datas->selected)->size.Y *= (1 + 1 * diff * dt); 
                }
            }
            prevWheel = wheel;
        }
    }
}
