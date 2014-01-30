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



#include "Polygon.h"
#include "base/Log.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/constants.hpp>

Polygon Polygon::create(Shape::Enum e) {
	Polygon p;
	switch (e) {
		case Shape::Square:
			// 2 triangles -> 6 indices
			p.indices.push_back(0);
            p.indices.push_back(1);
            p.indices.push_back(2);
			p.indices.push_back(3);
			// p.indices.push_back(3);
			// p.indices.push_back(2);
			// 2 triangles -> 4 vertices
			p.vertices.push_back(glm::vec2(-0.5, -0.5)); //a
			p.vertices.push_back(glm::vec2(0.5, -0.5)); // c
			p.vertices.push_back(glm::vec2(-0.5, 0.5)); // b
			p.vertices.push_back(glm::vec2(0.5, 0.5)); // d
			break;
		case Shape::Triangle:
			// 1 triangle -> 3 indices
			p.indices.push_back(0);
			p.indices.push_back(1);
			p.indices.push_back(2);
			// 1 triangle -> 1 vertices
			p.vertices.push_back(glm::vec2(-0.5, -0.5));
			p.vertices.push_back(glm::vec2(0.5, -0.5));
			p.vertices.push_back(glm::vec2(0, 0.5));
			break;
		case Shape::Hexagon:
            // top half
			p.indices.push_back(1);
            p.indices.push_back(2);
            p.indices.push_back(0);
            p.indices.push_back(3);
            p.indices.push_back(4);
            // bottom half
            p.indices.push_back(4);
            p.indices.push_back(5);
            p.indices.push_back(0);
            p.indices.push_back(6);
            p.indices.push_back(1);

			// 6 triangles -> 7 vertices
			p.vertices.push_back(glm::vec2(0));
			for (int i = 0; i<6; ++i) {
				p.vertices.push_back(glm::rotate(glm::vec2(0, 0.5),
					i * glm::pi<float>() / 3.0f));
			}
			break;
		default:
			LOGF("Shape::Enum not handled: " << e);
	}
	// LOGF_IF(p.indices.size() % 3, "Shapes are drawn as triangles, so indices count must be 3N. Invalid value for shape " << e << ":" << p.indices.size());
	return p;
}

    std::vector<glm::vec2> vertices;
    std::vector<unsigned short> indices;
