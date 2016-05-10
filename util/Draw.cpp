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
#include "util/MurmurHash.h"

#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>
#include "base/EntityManager.h"
#include "util/IntersectionUtil.h"

Draw Draw::instance;

static constexpr hash_t TempGroupId = Murmur::_Hash(__FILE__);

static std::vector<int> unusedRenderingEntities;
static std::vector<int> unusedTextEntities;

Entity Draw::renderingEntity(hash_t groupID) {
    Entity t = 0;

    if (unusedRenderingEntities.empty()) {
        t = theEntityManager.CreateEntity(HASH("__/draw_r", 0x222c4e96));
        ADD_COMPONENT(t, Transformation);
        ADD_COMPONENT(t, Rendering);
        TRANSFORM(t)->z = 0.95;
        RENDERING(t)->flags = RenderingFlags::NonOpaque;

        rendering.push_back(std::make_pair(t, groupID));
    } else {
        size_t s = unusedRenderingEntities.size();
        int index = unusedRenderingEntities[s - 1];
        auto* firstUnused = &rendering[index];
        firstUnused->second = groupID;
        t = firstUnused->first;
        unusedRenderingEntities.resize(s - 1);
    }
    return t;
}

Entity Draw::textEntity(hash_t groupID) {
    Entity t = 0;

    if (unusedTextEntities.empty()) {
        t = theEntityManager.CreateEntity(HASH("__/draw_t", 0xa021a4bd));
        ADD_COMPONENT(t, Transformation);
        ADD_COMPONENT(t, Text);

        text.push_back(std::make_pair(t, groupID));
    } else {
        size_t s = unusedTextEntities.size();
        int index = unusedTextEntities[s - 1];
        auto* firstUnused = &text[index];
        firstUnused->second = groupID;
        t = firstUnused->first;
        unusedTextEntities.resize(s - 1);
    }
    return t;
}

static void addText(Entity t, Entity parent, const std::string& text) {
    TEXT(t)->charHeight = 1.0f;
    TEXT(t)->text = text;
    TEXT(t)->color = Color(0,0,0);
    TEXT(t)->show = true;
    TRANSFORM(t)->z = 0.99f;
    TRANSFORM(t)->size = 2.0f * TRANSFORM(parent)->size;
    TRANSFORM(t)->position = TRANSFORM(parent)->position;
    TEXT(t)->flags = 0;
}

void Draw::Clear(hash_t groupID) {
    const size_t sR = instance.rendering.size();
    for (size_t i=0; i<sR; i++) {
        const auto& e = instance.rendering[i];
        if (e.second == groupID) {
            RENDERING(e.first)->show = false;
            unusedRenderingEntities.push_back(i);
        }
    }
    const size_t sT = instance.text.size();
    for (size_t i=0; i<sT; i++) {
        const auto& e = instance.text[i];
        if (e.second == groupID) {
            TEXT(e.first)->show = false;
            unusedTextEntities.push_back(i);
        }
    }
}

void Draw::ClearAll() {
    const size_t sR = instance.rendering.size();
    for (size_t i=0; i<sR; i++) {
        const auto& e = instance.rendering[i];
        RENDERING(e.first)->show = false;
        unusedRenderingEntities.push_back(i);
    }
    const size_t sT = instance.text.size();
    for (size_t i=0; i<sT; i++) {
        const auto& e = instance.text[i];
        TEXT(e.first)->show = false;
        unusedTextEntities.push_back(i);
    }
}

void Draw::Point(const glm::vec2& position, const Color & color, const std::string& text) {
    Point(TempGroupId, position, color, text);
}

void Draw::Point(hash_t groupID, const glm::vec2& position, const Color & color, const std::string& text) {
    Entity pt = instance.renderingEntity(groupID);

    TRANSFORM(pt)->size = glm::vec2(0.2f);
    TRANSFORM(pt)->position = position;
    TRANSFORM(pt)->rotation = 0.0f;

    RENDERING(pt)->color = color;
    RENDERING(pt)->show = true;

    if (!text.empty()) {
        Entity tx = instance.textEntity(groupID);
        addText(tx, pt, text);
        TEXT(tx)->flags = TextComponent::AdjustHeightToFillWidthBit;
    }
}

void Draw::Vec2(const glm::vec2& position, const glm::vec2& size, const Color & color, const std::string& text) {
    Vec2(TempGroupId, position, size, color, text);
}

void Draw::Vec2(hash_t groupID, const glm::vec2& position, const glm::vec2& size, const Color & color, const std::string& text) {
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
Entity Draw::Triangle(hash_t groupID, const glm::vec2& firstPoint, const glm::vec2& secondPoint, const glm::vec2& thirdPoint,
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

void Draw::RectangleAABB(const AABB& aabb,
        const Color& color,
        const std::string& text) {
    Rectangle(TempGroupId,
        glm::vec2(
            (aabb.right + aabb.left) * 0.5f,
            (aabb.top + aabb.bottom) * 0.5f),
        glm::vec2(
            aabb.right - aabb.left,
            aabb.top - aabb.bottom),
        0.0f, color, text);
}

void Draw::Rectangle(const glm::vec2& centerPosition, const glm::vec2& size, float rotation, const Color & color,
    const std::string& text) {
    Rectangle(TempGroupId, centerPosition, size, rotation, color, text);
}
void Draw::Rectangle(hash_t groupID, const glm::vec2& centerPosition, const glm::vec2& size, float rotation, const Color & color,
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

void Draw::Update() {
    Clear(TempGroupId);
}
