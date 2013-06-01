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
			p.indices.push_back(1);
			p.indices.push_back(3);
			p.indices.push_back(2);
			// 2 triangles -> 4 vertices
			p.vertices.push_back(glm::vec2(-0.5, -0.5));
			p.vertices.push_back(glm::vec2(0.5, -0.5));
			p.vertices.push_back(glm::vec2(-0.5, 0.5));
			p.vertices.push_back(glm::vec2(0.5, 0.5));
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
			// 6 triangles -> 18 indices
			for (int i = 1; i <= 6; ++i) {
				p.indices.push_back(0);
				p.indices.push_back(i);
				p.indices.push_back((i < 6) ? (i + 1) : 1);
			}
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
	LOGF_IF(p.indices.size() % 3, "Shapes are drawn as triangles, so indices count must be 3N. Invalid value for shape " << e << ":" << p.indices.size());
	return p;
}

    std::vector<glm::vec2> vertices;
    std::vector<unsigned short> indices;
