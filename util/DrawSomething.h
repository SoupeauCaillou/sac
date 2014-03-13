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

#include "base/Entity.h"
#include "base/Color.h"

#include <glm/glm.hpp>

#include <vector>

class DrawSomething {
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


        static Entity DrawRectangle(const std::string& groupID, const glm::vec2& centerPosition, const glm::vec2& size,
            float rotation, const Color & color = Color(), const std::string name = "rectangle", Entity vector = 0);
        static void DrawRectangleRestart(const std::string & groupID);


        static void Clear();
    private:
        static DrawSomething instance;
        DrawSomething() { drawPointList.clear(); drawVec2List.clear(); drawTriangleList.clear(); drawRectangleList.clear(); }

        std::vector<std::pair<Entity, std::string>> drawPointList;
        std::vector<std::pair<Entity, std::string>> drawVec2List;
        std::vector<std::pair<Entity, std::string>> drawTriangleList;
        std::vector<std::pair<Entity, std::string>> drawRectangleList;
};

