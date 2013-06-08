#include "DrawSomething.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

#include "base/Log.h"

#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>

Draw Draw::instance;

Entity Draw::DrawPoint(const std::string& groupID, const glm::vec2& position,
 const Color & color, const std::string name, Entity vector) {
    if (vector == 0) {
        auto firstUnused = instance.drawPointList.begin();
        for (; firstUnused != instance.drawPointList.end(); ++firstUnused) {
            if (RENDERING(firstUnused->first)->show == false) {
                break;
            }
        }

        if (firstUnused == instance.drawPointList.end()) {
            vector = theEntityManager.CreateEntity(name);
            ADD_COMPONENT(vector, Transformation);
            ADD_COMPONENT(vector, Rendering);

            TRANSFORM(vector)->z = 1.;

            instance.drawPointList.push_back(std::make_pair(vector, groupID));
            vector = instance.drawPointList.back().first;
        } else {
            vector = firstUnused->first;
        }
    }

    TRANSFORM(vector)->size = glm::vec2(0.5f);
    TRANSFORM(vector)->position = position;

    RENDERING(vector)->color = color;//Color(.5, currentDrawPointIndice * 1.f / list.size(), currentDrawPointIndice * 1.f / list.size());
    RENDERING(vector)->show = true;

    return vector;
}
void Draw::DrawPointRestart(const std::string & groupID) {
    for (auto e : instance.drawPointList) {
        if (e.second == groupID) {
            RENDERING(e.first)->show = false;
        }
    }
}




Entity Draw::DrawVec2(const std::string& groupID, const glm::vec2& position, const glm::vec2& size,
 bool useTexture, const std::string name, Entity vector) {
    Entity e = DrawVec2(groupID, position, size, Color(), name, vector);
    if (useTexture) {
        RENDERING(e)->texture = theRenderingSystem.loadTextureFile("fleche");
    }

    return e;
}

Entity Draw::DrawVec2(const std::string& groupID, const glm::vec2& position, const glm::vec2& size,
 const Color & color, const std::string name, Entity vector) {
    if (vector == 0) {
        auto firstUnused = instance.drawVec2List.begin();
        for (; firstUnused != instance.drawVec2List.end(); ++firstUnused) {
            if (RENDERING(firstUnused->first)->show ==false) {
                break;
            }
        }

        if (firstUnused == instance.drawVec2List.end()) {
            vector = theEntityManager.CreateEntity(name);
            ADD_COMPONENT(vector, Transformation);
            ADD_COMPONENT(vector, Rendering);

            TRANSFORM(vector)->z = 1.;

            instance.drawVec2List.push_back(std::make_pair(vector, groupID));
           vector = instance.drawVec2List.back().first;
        } else {
            vector = firstUnused->first;
        }
    }

    TRANSFORM(vector)->size = glm::vec2(glm::length(size), .3f);
    TRANSFORM(vector)->rotation = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(size));
    //LOGV(1, "normalize : " << glm::normalize(size).x << "," << glm::normalize(size).y << " : " << glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(size)));

    float y = TRANSFORM(vector)->size.x * glm::sin(TRANSFORM(vector)->rotation);
    float x = TRANSFORM(vector)->size.x * glm::cos(TRANSFORM(vector)->rotation);
    TRANSFORM(vector)->position = position + glm::vec2(x, y) / 2.f;
    //LOGV(1, "Vector " << vector << ": " << TRANSFORM(vector)->position.x << "," << TRANSFORM(vector)->position.y << " : " << TRANSFORM(vector)->size.x << "," << TRANSFORM(vector)->size.y << " : " << TRANSFORM(vector)->rotation);

    RENDERING(vector)->color = color;
    RENDERING(vector)->show = true;

    return vector;
}
void Draw::DrawVec2Restart(const std::string & groupID) {
    for (auto e : instance.drawVec2List) {
        if (e.second == groupID) {
            RENDERING(e.first)->show = false;
        }
    }
}


Entity Draw::DrawTriangle(const std::string& groupID, const glm::vec2& firstPoint, const glm::vec2& secondPoint, const glm::vec2& thirdPoint,
 const Color & color, const std::string name, Entity vector, int dynamicVertices) {
    if (vector == 0) {
        auto firstUnused = instance.drawTriangleList.begin();
        int i = 0;
        for (; firstUnused != instance.drawTriangleList.end(); ++firstUnused) {

            if (RENDERING(firstUnused->first)->show ==false) {
                break;
            }
            ++i;
        }

        if (firstUnused == instance.drawTriangleList.end()) {
            vector = theEntityManager.CreateEntity(name);
            ADD_COMPONENT(vector, Transformation);
            ADD_COMPONENT(vector, Rendering);

            TRANSFORM(vector)->z = 1.;

            dynamicVertices = instance.drawTriangleList.size();
            instance.drawTriangleList.push_back(std::make_pair(vector, groupID));

            vector = instance.drawTriangleList.back().first;
        } else {
            dynamicVertices = firstUnused - instance.drawTriangleList.begin();
            LOGF_IF(i != dynamicVertices, i << " vs " << dynamicVertices);
            vector = firstUnused->first;
        }
    }
    TRANSFORM(vector)->position = glm::vec2(0.);
    TRANSFORM(vector)->size = glm::vec2(1.f);
    RENDERING(vector)->shape = Shape::Triangle;
    RENDERING(vector)->color = color;
    RENDERING(vector)->dynamicVertices = dynamicVertices;

    std::vector<glm::vec2> vert;
    vert.push_back(firstPoint);
    vert.push_back(secondPoint);
    vert.push_back(thirdPoint);
    theRenderingSystem.defineDynamicVertices(RENDERING(vector)->dynamicVertices, vert);
    RENDERING(vector)->show = true;


    return vector;
}
void Draw::DrawTriangleRestart(const std::string & groupID) {
    for (auto e : instance.drawTriangleList) {
        if (e.second == groupID) {
            RENDERING(e.first)->show = false;
        }
    }
}
