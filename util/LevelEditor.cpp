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

struct LevelEditor::LevelEditorDatas {
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
    if (!theTouchInputManager.isTouched(0) && theTouchInputManager.wasTouched(0)) {
        Vector2 position = theTouchInputManager.getTouchLastPosition(0);
        // adjust position depending on current camera...
        const Vector2 windowPos = Vector2(position.X / PlacementHelper::ScreenWidth, position.Y / PlacementHelper::ScreenHeight);
        for (unsigned i=0; i<theRenderingSystem.cameras.size(); i++) {
            const RenderingSystem::Camera& cam = theRenderingSystem.cameras[i];
            if (IntersectionUtil::pointRectangle(windowPos, cam.screenPosition, cam.screenSize) && cam.enable) {
                position += cam.worldPosition;
                break;
            }
        }
        if (datas->selected) {
            if (!IntersectionUtil::pointRectangle(position, TRANSFORM(datas->selected)->worldPosition, TRANSFORM(datas->selected)->size)) {
                deselect(datas->selected);
                datas->selected = 0;
            }
        } else {
            std::vector<Entity> entities = theRenderingSystem.RetrieveAllEntityWithComponent();
            float nearest = 10000;
            for (unsigned i=0; i<entities.size(); i++) {
                // select(entities[i]);
                if (IntersectionUtil::pointRectangle(position, TRANSFORM(entities[i])->worldPosition, TRANSFORM(entities[i])->size)) {
                    float d = Vector2::DistanceSquared(position, TRANSFORM(entities[i])->worldPosition);
                    if (d < nearest) {
                        datas->selected = entities[i];
                        nearest = d;
                    }
                }
            }
            if (datas->selected)
                select(datas->selected);
        }
    }
}
