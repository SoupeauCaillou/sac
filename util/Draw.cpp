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



#include "Draw.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TextSystem.h"
#include "systems/AnchorSystem.h"

#include "base/Log.h"

#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>

Draw Draw::instance;

Entity Draw::renderingEntity(const std::string& groupID) {
    Entity t = 0;
    auto firstUnused = rendering.begin();
    for (; firstUnused != rendering.end(); ++firstUnused) {
        if (RENDERING(firstUnused->first)->show ==false) {
            break;
        }
    }
    if (firstUnused == rendering.end()) {
        t = theEntityManager.CreateEntity("__draw");
        ADD_COMPONENT(t, Transformation);
        ADD_COMPONENT(t, Rendering);
        TRANSFORM(t)->z = 0.9;
        RENDERING(t)->opaqueType = RenderingComponent::NON_OPAQUE;

        rendering.push_back(std::make_pair(t, groupID));
    } else {
        firstUnused->second = groupID;
        t = firstUnused->first;
    }
    return t;
}

Entity Draw::textEntity(const std::string& groupID) {
    Entity t = 0;
    auto firstUnused = text.begin();
    for (; firstUnused != text.end(); ++firstUnused) {
        if (TEXT(firstUnused->first)->show ==false) {
            break;
        }
    }
    if (firstUnused == text.end()) {
        t = theEntityManager.CreateEntity("__draw");
        ADD_COMPONENT(t, Transformation);
        ADD_COMPONENT(t, Text);

        text.push_back(std::make_pair(t, groupID));
    } else {
        firstUnused->second = groupID;
        t = firstUnused->first;
    }
    return t;
}

static void addText(Entity t, Entity parent, const std::string& text) {
    AnchorComponent c;
    c.position = glm::vec2(0.0f, TEXT(t)->charHeight * 0.5);
    AnchorSystem::adjustTransformWithAnchor(TRANSFORM(t), TRANSFORM(parent), &c);
    TEXT(t)->charHeight = glm::min(TRANSFORM(parent)->size.x, 0.5f);
    TEXT(t)->text = text;
    TEXT(t)->color = Color(0,0,0);
    TEXT(t)->show = true;
}

void Draw::Clear(const std::string & groupID) {
    for (auto e : instance.rendering) {
        if (e.second == groupID) {
            RENDERING(e.first)->show = false;
        }
    }
    for (auto e : instance.text) {
        if (e.second == groupID) {
            TEXT(e.first)->show = false;
        }
    }
}

void Draw::ClearAll() {
    for (auto item : instance.rendering) {
        theEntityManager.DeleteEntity(item.first);
    }
    instance.rendering.clear();

    for (auto item : instance.text) {
        theEntityManager.DeleteEntity(item.first);
    }
    instance.text.clear();
}

void Draw::Point(const std::string& groupID, const glm::vec2& position, const Color & color, const std::string& text) {
    Entity pt = instance.renderingEntity(groupID);

    TRANSFORM(pt)->size = glm::vec2(0.2f);
    TRANSFORM(pt)->position = position;

    RENDERING(pt)->color = color;
    RENDERING(pt)->show = true;

    if (!text.empty()) {
        addText(instance.textEntity(groupID), pt, text);
    }
}

void Draw::Vec2(const std::string& groupID, const glm::vec2& position, const glm::vec2& size, const Color & color, const std::string& text) {
    Entity vector = instance.renderingEntity(groupID);

    TRANSFORM(vector)->size = glm::vec2(glm::length(size), .05f);
    TRANSFORM(vector)->rotation = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(size));

    float y = TRANSFORM(vector)->size.x * glm::sin(TRANSFORM(vector)->rotation);
    float x = TRANSFORM(vector)->size.x * glm::cos(TRANSFORM(vector)->rotation);
    TRANSFORM(vector)->position = position + glm::vec2(x, y) / 2.f;

    RENDERING(vector)->color = color;
    RENDERING(vector)->show = true;

    if (!text.empty()) {
        addText(instance.textEntity(groupID), vector, text);
    }
}

#if 0
Entity Draw::Triangle(const std::string& groupID, const glm::vec2& firstPoint, const glm::vec2& secondPoint, const glm::vec2& thirdPoint,
 const Color & color, const std::string name, Entity vector, int dynamicVertices) {
    Entity triangle = renderingEntity(groupID);

    TRANSFORM(triangle)->position = glm::vec2(0.);
    TRANSFORM(triangle)->size = glm::vec2(1.f);
    RENDERING(triangle)->shape = Shape::Triangle;
    RENDERING(triangle)->color = color;
    RENDERING(triangle)->dynamicVertices = dynamicVertices;

    std::vector<glm::vec2> vert;
    vert.push_back(firstPoint);
    vert.push_back(secondPoint);
    vert.push_back(thirdPoint);
    theRenderingSystem.defineDynamicVertices(RENDERING(vector)->dynamicVertices, vert);
    RENDERING(triangle)->show = true;


    return vector;
}
#endif

void Draw::Rectangle(const std::string& groupID, const glm::vec2& centerPosition, const glm::vec2& size, float rotation, const Color & color,
    const std::string& text) {

    Entity rect = instance.renderingEntity(groupID);

    TRANSFORM(rect)->position = centerPosition;
    TRANSFORM(rect)->size = size;
    TRANSFORM(rect)->rotation = rotation;
/*
    float y = TRANSFORM(rect)->size.x * glm::sin(TRANSFORM(rect)->rotation);
    float x = TRANSFORM(rect)->size.x * glm::cos(TRANSFORM(rect)->rotation);
    TRANSFORM(rect)->position = centerPosition + glm::vec2(x, y) / 2.f;
*/
    RENDERING(rect)->color = color;
    RENDERING(rect)->show = true;

    if (!text.empty()) {
        addText(instance.textEntity(groupID), rect, text);
    }
}
