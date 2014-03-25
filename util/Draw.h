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

class Draw {
    public:
        static void Point(uint32_t permanentGroupID,
            const glm::vec2& position, const Color & color = Color(), const std::string& text = "");
        static void Point(
            const glm::vec2& position, const Color & color = Color(), const std::string& text = "");

        static void Vec2(uint32_t permanentGroupID,
            const glm::vec2& position, const glm::vec2& size, const Color & color = Color(), const std::string& text = "");

        static void Vec2(
            const glm::vec2& position, const glm::vec2& size, const Color & color = Color(), const std::string& text = "");

#if 0
        static Entity Triangle(const std::string& permanentGroupID, const glm::vec2& firstPoint, const glm::vec2& secondPoint, const glm::vec2& thirdPoint,
            const Color & color = Color(), const std::string& name = "triangle", Entity vector = 0, int dynamicVertices = 0);
#endif

        static void Rectangle(uint32_t permanentGroupID,
            const glm::vec2& centerPosition, const glm::vec2& size, float rotation, const Color & color = Color(), const std::string& text = "");
        static void Rectangle(
            const glm::vec2& centerPosition, const glm::vec2& size, float rotation, const Color & color = Color(), const std::string& text = "");

        static void Update();

        static void Clear(uint32_t permanentGroupID);

        static void ClearAll();
    private:
        static Draw instance;

        Entity renderingEntity(uint32_t groupID);
        Entity textEntity(uint32_t groupID);
        std::vector<std::pair<Entity, uint32_t>> rendering;
        std::vector<std::pair<Entity, uint32_t>> text;
};

