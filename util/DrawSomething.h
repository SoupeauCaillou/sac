#pragma once

#include "base/Entity.h"
#include "base/Color.h"

#include <glm/glm.hpp>

#include <vector>

class Draw {
    public:
        static Entity DrawPoint(const std::string& groupID, const glm::vec2& position,
            const Color & color = Color(), const std::string name = "point", Entity vector = 0);
        static void DrawPointRestart(const std::string & groupID);


        static Entity DrawVec2(const std::string& groupID, const glm::vec2& position, const glm::vec2& size,
            const Color & color = Color(), const std::string name = "glm::vec2", Entity vector = 0);
        static Entity DrawVec2(const std::string& groupID, const glm::vec2& position, const glm::vec2& size,
            bool useTexture, const std::string name = "glm::vec2", Entity vector = 0);
        static void DrawVec2Restart(const std::string & groupID);

        static Entity DrawTriangle(const std::string& groupID, const glm::vec2& firstPoint, const glm::vec2& secondPoint, const glm::vec2& thirdPoint,
            const Color & color = Color(), const std::string name = "triangle", Entity vector = 0, int dynamicVertices = 0);
        static void DrawTriangleRestart(const std::string & groupID);

    private:
        static Draw instance;
        Draw() {}

        std::vector<std::pair<Entity, std::string>> drawPointList;
        std::vector<std::pair<Entity, std::string>> drawVec2List;
        std::vector<std::pair<Entity, std::string>> drawTriangleList;
};

