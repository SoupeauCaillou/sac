#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"

#include "base/Log.h"

#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>

Entity drawVector(const glm::vec2& position, const glm::vec2& size) {
    Entity vector = theEntityManager.CreateEntity("vector");
    ADD_COMPONENT(vector, Transformation);
    ADD_COMPONENT(vector, Rendering);

    TRANSFORM(vector)->size = glm::vec2(glm::length(size), 0.1f);
    TRANSFORM(vector)->rotation = glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(size));
    //LOGV(1, "normalize : " << glm::normalize(size).x << "," << glm::normalize(size).y << " : " << glm::orientedAngle(glm::vec2(1.f, 0.f), glm::normalize(size)));

    float y = TRANSFORM(vector)->size.x * glm::sin(TRANSFORM(vector)->rotation);
    float x = TRANSFORM(vector)->size.x * glm::cos(TRANSFORM(vector)->rotation);
    TRANSFORM(vector)->position = position + glm::vec2(x, y) / 2.f;
    //LOGV(1, "Vector " << vector << ": " << TRANSFORM(vector)->position.x << "," << TRANSFORM(vector)->position.y << " : " << TRANSFORM(vector)->size.x << "," << TRANSFORM(vector)->size.y << " : " << TRANSFORM(vector)->rotation);
    TRANSFORM(vector)->z = 1;
    RENDERING(vector)->texture = theRenderingSystem.loadTextureFile("fleche");
    RENDERING(vector)->show = true;

    return vector;
}
